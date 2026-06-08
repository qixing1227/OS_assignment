#include "smart_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

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

// 运行智能 Shell 
void run_smart_shell() {
    // 【系统守护】：忽略 Ctrl+C (SIGINT) 信号，防止智能 Shell 本体被用户误杀
    signal(SIGINT, SIG_IGN);

    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    printf("\n======================================================\n");
    printf("   欢迎进入 OS 控制台 (主入口)\n");
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

        strcpy(final_command, input);

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
