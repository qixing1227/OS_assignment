#include "common.h"
#include "scheduler.h"
#include "memory_manager.h"
#include "sync_concurrency.h"
#include "file_system.h"
#include "smart_shell.h"

void run_task1_scheduler() {
/* Lines 4-46 unchanged */
    Process *processes = NULL;
    int n = 0;

    printf("\n=== 任务一：处理机调度 (Processor Scheduling) ===\n");
    input_processes(&processes, &n);
    if (n <= 0) {
        printf("进程数量无效，返回主菜单。\n");
        return;
    }

    int choice;
    while(1) {
        printf("\n请选择调度算法：\n");
        printf("1. FCFS (先来先服务)\n");
        printf("2. SJF (短作业优先 - 非抢占式)\n");
        printf("3. RR (时间片轮转)\n");
        printf("0. 退出处理机调度模块\n");
        printf("请输入您的选择: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            fcfs(processes, n);
        } else if (choice == 2) {
            sjf(processes, n);
        } else if (choice == 3) {
            int time_quantum;
            printf("请输入时间片大小(Time Quantum): ");
            scanf("%d", &time_quantum);
            if(time_quantum <= 0) {
                printf("时间片必须大于0\n");
            } else {
                rr(processes, n, time_quantum);
            }
        } else if (choice == 0) {
            break;
        } else {
            printf("无效的选择，请重新输入！\n");
        }
    }

    if (processes) {
        free(processes);
    }
}

int main() {
    printf("欢迎运行操作系统课程设计项目 (OS Course Design)\n");
    
    int main_choice;
    while(1) {
        printf("\n====== 主菜单 ======\n");
        printf("1. 任务一：处理机调度算法实验\n");
        printf("2. 任务二：内存管理模块实验\n");
        printf("3. 任务三：进程同步与并发控制\n");
        printf("4. 任务四：简单文件系统模拟\n");
        printf("5. 扩展实验：大模型驱动 SmartShell 原型\n");
        printf("0. 退出系统\n");
        printf("请输入您的选择: ");
        if (scanf("%d", &main_choice) != 1) break;

        // 清除缓冲区的换行符，防止后续 fgets 直接跳过
        while(getchar() != '\n');

        if (main_choice == 1) {
            run_task1_scheduler();
        } else if (main_choice == 2) {
            run_task2_memory_management();
        } else if (main_choice == 3) {
            run_task3_sync_concurrency();
        } else if (main_choice == 4) {
            run_task4_file_system();
        } else if (main_choice == 5) {
            run_smart_shell();
        } else if (main_choice == 0) {
            printf("退出程序。\n");
            break;
        } else {
            printf("无效的选择！\n");
        }
    }
    return 0;
}