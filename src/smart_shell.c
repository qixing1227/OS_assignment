#include "smart_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

// 辅助函数：将字符串按空格拆分成参数数组（供 execvp 使用）
void parse_command(char *cmd, char **args) {
    int i = 0;
    args[i] = strtok(cmd, " \n");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok(NULL, " \n");
    }
    args[i] = NULL;
}

// 核心功能：调用 Python 桥接层获取 AI 返回的命令
void ask_ai_for_command(const char *user_intent, char *ai_command) {
    char python_call[MAX_INPUT_SIZE + 50];
    
    // 组装调用命令：调用 scripts/ 目录下的原生接口桥接代码
    snprintf(python_call, sizeof(python_call), "python3 scripts/llm_bridge.py \"%s\"", user_intent);

    // popen: 创建一个管道，执行外部程序并读取其标准输出
    FILE *fp = popen(python_call, "r");
    if (fp == NULL) {
        strcpy(ai_command, "Error: AI 调用进程启动失败");
        return;
    }

    // 读取 AI 返回的第一行内容（即生成的命令）
    if (fgets(ai_command, MAX_INPUT_SIZE, fp) != NULL) {
        ai_command[strcspn(ai_command, "\n")] = 0; // 去除换行符
    } else {
        strcpy(ai_command, "Error: AI 系统没有返回任何数据");
    }
    pclose(fp);
}

// 运行智能 Shell 
void run_smart_shell() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    printf("\n======================================================\n");
    printf("   欢迎进入 SmartOS 智能控制台 (输入 'exit' 退出)\n");
    printf("   引擎: \033[1;36mQwen 2.5 : 7B\033[0m 本地驱动支持\n");
    printf("   提示: 以 '@' 开头输入自然语言，即可唤醒 AI 助手\n");
    printf("======================================================\n");

    while (1) {
        printf("\033[1;32mSmartOS > \033[0m"); // 绿色的提示符
        fflush(stdout);

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = 0; // 移除换行符

        if (strlen(input) == 0) continue;
        if (strcmp(input, "exit") == 0) break;

        char final_command[MAX_INPUT_SIZE];

        // ================= 1. AI 意图解析模式 =================
        if (input[0] == '@') {
            char *intent = input + 1; // 跳过 '@'
            printf("[\033[1;36mAI思考中(Qwen2.5:7b)...\033[0m] 正在分析您的意图: %s\n", intent);
            
            ask_ai_for_command(intent, final_command);
            
            if (strncmp(final_command, "Error", 5) == 0) {
                printf("[\033[1;31mAI系统异常\033[0m] %s\n", final_command);
                continue;
            }

            // 【创新点】：沙箱安全确认机制
            printf("[\033[1;33m安全拦截\033[0m] AI 生成的底层调用指令为: \033[1;35m%s\033[0m\n", final_command);
            printf("是否允许执行此命令? (y/n): ");
            char confirm = getchar();
            while (getchar() != '\n'); // 清空输入缓冲区

            if (confirm != 'y' && confirm != 'Y') {
                printf("[\033[1;31m拦截\033[0m] 操作已取消。\n");
                continue;
            }
            printf("[\033[1;32m放行\033[0m] 开始执行OS原生调用...\n");
        } 
        // ================= 2. 传统 Shell 模式 =================
        else {
            strcpy(final_command, input);
        }

        // ================= 3. 操作系统底层机制：创建子进程执行命令 =================
        // 为了支持内置命令，我们需要一份拷贝来做参数分割
        char cmd_copy[MAX_INPUT_SIZE];
        strcpy(cmd_copy, final_command);

        parse_command(cmd_copy, args);
        if (args[0] == NULL) continue;

        // 内置命令 cd 需要特殊处理 (cd 不能通过 fork 执行，因为要改变父进程本身的环境)
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL && chdir(args[1]) != 0) {
                perror("cd 失败");
            }
            continue;
        }

        // 核心：使用 fork 创建子进程
        pid_t pid = fork();
        if (pid < 0) {
            perror("进程创建失败");
        } else if (pid == 0) {
            // 子进程：替换内存映像以执行新程序
            // 修复经典问题：execvp(args[0], args) 默认不懂什么是管道符 `|`，它会把 `|` 当成普通参数！
            // 为了完美支持大模型解析出的含有 `|` 和 `>` 的复杂复合型指令，我们调用操作系统的经典 shell 帮忙代理执行
            char *sh_args[] = {"sh", "-c", final_command, NULL};
            if (execvp("sh", sh_args) == -1) {
                printf("未知指令无法执行: %s\n", final_command);
                exit(EXIT_FAILURE);
            }
        } else {
            // 父进程：等待子进程执行完毕
            int status;
            waitpid(pid, &status, 0);
        }
    }
    printf("退出 SmartOS，感谢使用！\n");
}
