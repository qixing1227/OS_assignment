#include "smart_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

// 引入 OS 内置实验模块
extern void run_task1_scheduler();
extern void run_task2_memory_management();
extern void run_task3_sync_concurrency();
extern void run_task4_file_system();

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

// 【安全补丁】：对自然语言输入进行字符逃逸/净空，防止跨站命令注入(Command Injection)
void sanitize_input(const char* src, char* dst) {
    int j = 0;
    for(int i = 0; src[i] != '\0' && j < MAX_INPUT_SIZE - 2; i++) {
        char c = src[i];
        // 抹除极其危险的符号
        if(c == '"' || c == '\'' || c == '\\' || c == ';' || c == '&' || c == '|' || c == '$' || c == '`') {
            dst[j++] = ' ';
        } else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

// 核心功能：调用 Python 桥接层获取 AI 返回的命令
void ask_ai_for_command(const char *user_intent, char *ai_command) {
    char python_call[MAX_INPUT_SIZE + 50];
    char safe_intent[MAX_INPUT_SIZE];
    
    // 注入防御：清洗危险字符
    sanitize_input(user_intent, safe_intent);
    
    snprintf(python_call, sizeof(python_call), "python3 scripts/llm_bridge.py \"%s\"", safe_intent);

    FILE *fp = popen(python_call, "r");
    if (fp == NULL) {
        strcpy(ai_command, "Error: AI 调用进程启动失败");
        return;
    }

    if (fgets(ai_command, MAX_INPUT_SIZE, fp) != NULL) {
        ai_command[strcspn(ai_command, "\n")] = 0;
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
    printf("   欢迎进入 SmartOS 智能控制台 (唯一主入口)\n");
    printf("   引擎: \033[1;36mQwen 2.5 : 7B\033[0m 搭载智能体沙箱防御系统\n");
    printf("   提示: 以 '@' 开头输入自然语言，即可唤醒 AI 助手\n");
    printf("   内置模块扩展: run_scheduler, run_memory, run_concurrency, run_fs\n");
    printf("======================================================\n");

    while (1) {
        printf("\033[1;32mSmartOS > \033[0m"); 
        fflush(stdout);

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = 0; 
        if (strlen(input) == 0) continue;
        if (strcmp(input, "exit") == 0) break;

        char final_command[MAX_INPUT_SIZE];

        if (input[0] == '@') {
            char *intent = input + 1;
            printf("[\033[1;36mAI思考中(Qwen)... \033[0m] 解析意图: %s\n", intent);
            
            ask_ai_for_command(intent, final_command);
            if (strncmp(final_command, "Error", 5) == 0) {
                printf("[\033[1;31m异常\033[0m] %s\n", final_command);
                continue;
            }

            printf("[\033[1;33m安全拦截\033[0m] AI 指令: \033[1;35m%s\033[0m\n", final_command);
            printf("是否允许执行? (y/n): ");
            char confirm = getchar();
            while (getchar() != '\n'); 

            if (confirm != 'y' && confirm != 'Y') {
                printf("[\033[1;31m拦截\033[0m] 操作取消。\n");
                continue;
            }
        } else {
            strcpy(final_command, input);
        }

        // ================= 内置子系统劫持执行 =================
        char cmd_copy[MAX_INPUT_SIZE];
        strcpy(cmd_copy, final_command);
        parse_command(cmd_copy, args);
        if (args[0] == NULL) continue;
        
        if (strcmp(args[0], "run_scheduler") == 0)   { run_task1_scheduler(); continue; }
        if (strcmp(args[0], "run_memory") == 0)      { run_task2_memory_management(); continue; }
        if (strcmp(args[0], "run_concurrency") == 0) { run_task3_sync_concurrency(); continue; }
        if (strcmp(args[0], "run_fs") == 0)          { run_task4_file_system(); continue; }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL && chdir(args[1]) != 0) { perror("cd 失败"); }
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("进程创建失败");
        } else if (pid == 0) {
            char *sh_args[] = {"sh", "-c", final_command, NULL};
            if (execvp("sh", sh_args) == -1) {
                printf("未知指令无法执行: %s\n", final_command);
                exit(EXIT_FAILURE);
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}
