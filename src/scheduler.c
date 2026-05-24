#include "scheduler.h"

// 辅助函数：重置进程统计与状态，方便复用同一组数据测试不同算法
void reset_processes(Process processes[], int n) {
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
    }
}

// 辅助函数：输出进程运行与调度统计指标
void print_processes(Process processes[], int n) {
    float total_wt = 0, total_tat = 0;
    printf("\n%-6s %-10s %-10s %-10s %-12s %-12s %-12s\n", 
           "PID", "Arrival", "Burst", "Priority", "Completion", "Turnaround", "Waiting");
    
    for (int i = 0; i < n; i++) {
        total_wt += processes[i].waiting_time;
        total_tat += processes[i].turnaround_time;
        printf("%-6d %-10d %-10d %-10d %-12d %-12d %-12d\n",
               processes[i].pid, 
               processes[i].arrival_time, 
               processes[i].burst_time,
               processes[i].priority, 
               processes[i].completion_time,
               processes[i].turnaround_time, 
               processes[i].waiting_time);
    }
    printf("\n[统计结果]\n");
    printf("平均等待时间 (Average Waiting Time)    : %.2f\n", total_wt / n);
    printf("平均周转时间 (Average Turnaround Time) : %.2f\n\n", total_tat / n);
}

// 辅助函数：用户动态输入进程参数
void input_processes(Process **processes_out, int *n_out) {
    int n;
    printf("请输入进程数量: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        *n_out = 0;
        return;
    }

    Process *processes = (Process *)malloc(sizeof(Process) * n);
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("请输入 进程 P%d 的 [到达时间] [执行时间(Burst)] [优先级]: ", i + 1);
        scanf("%d %d %d", &processes[i].arrival_time, &processes[i].burst_time, &processes[i].priority);
    }
    *processes_out = processes;
    *n_out = n;
}

// 辅助函数：按到达时间升序排序（冒泡算法）
void sort_by_arrival(Process processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j+1].arrival_time) {
                Process temp = processes[j];
                processes[j] = processes[j+1];
                processes[j+1] = temp;
            }
        }
    }
}

// ============================================
// 1. 先来先服务算法 (FCFS)
// ============================================
void fcfs(Process processes_orig[], int n) {
    // 拷贝一份数据进行排序，避免打乱原始数组（如果需要的话，这里直接原址修改以便后续print）
    sort_by_arrival(processes_orig, n);
    reset_processes(processes_orig, n);

    int current_time = 0;
    
    printf("\n=== [FCFS] 先来先服务调度执行顺序 ===\n");
    for (int i = 0; i < n; i++) {
        // 如果当前时间小于进程到达时间，CPU会出现空闲
        if (current_time < processes_orig[i].arrival_time) {
            printf("[时间 %2d - %2d] CPU 空闲\n", current_time, processes_orig[i].arrival_time);
            current_time = processes_orig[i].arrival_time;
        }
        
        processes_orig[i].start_time = current_time;
        current_time += processes_orig[i].burst_time;
        processes_orig[i].completion_time = current_time;
        
        processes_orig[i].turnaround_time = processes_orig[i].completion_time - processes_orig[i].arrival_time;
        processes_orig[i].waiting_time = processes_orig[i].turnaround_time - processes_orig[i].burst_time;
        
        printf("[时间 %2d - %2d] 进程 P%d 正在运行\n", processes_orig[i].start_time, processes_orig[i].completion_time, processes_orig[i].pid);
    }
    
    print_processes(processes_orig, n);
}

// ============================================
// 2. 短作业优先算法 (SJF - 非抢占式)
// ============================================
void sjf(Process processes[], int n) {
    reset_processes(processes, n);
    
    int current_time = 0;
    int completed_count = 0;
    bool is_completed[1000] = {false}; // 假设最多1000个进程
    
    printf("\n=== [SJF] 短作业优先调度执行顺序 ===\n");
    
    while (completed_count < n) {
        int idx = -1;
        int min_burst = 99999999;
        
        // 找出所有已到达且未完成的进程中，执行时间最短的
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && !is_completed[i]) {
                if (processes[i].burst_time < min_burst) {
                    min_burst = processes[i].burst_time;
                    idx = i;
                }
                // 当执行时长相同时，考虑先到达的（FCFS法则补充）
                else if (processes[i].burst_time == min_burst) {
                    if (processes[i].arrival_time < processes[idx].arrival_time) {
                        idx = i;
                    }
                }
            }
        }
        
        if (idx != -1) {
            // 找到了可以执行的进程
            processes[idx].start_time = current_time;
            current_time += processes[idx].burst_time;
            processes[idx].completion_time = current_time;
            
            processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            
            is_completed[idx] = true;
            completed_count++;
            
            printf("[时间 %2d - %2d] 进程 P%d 正在运行\n", processes[idx].start_time, processes[idx].completion_time, processes[idx].pid);
        } else {
            // 没有已到达的进程，CPU空闲，向后推移一个时间单位
            // 简单处理也可以直接跳转到下一个最近的到达时间
            int next_arrival = 99999999;
            for (int i = 0; i < n; i++) {
                if (!is_completed[i] && processes[i].arrival_time > current_time) {
                    if (processes[i].arrival_time < next_arrival) {
                        next_arrival = processes[i].arrival_time;
                    }
                }
            }
            printf("[时间 %2d - %2d] CPU 空闲\n", current_time, next_arrival);
            current_time = next_arrival;
        }
    }
    
    print_processes(processes, n);
}

// ============================================
// 3. 时间片轮转调度算法 (RR)
// ============================================
void rr(Process processes[], int n, int time_quantum) {
    // 采用简单的队列实现
    sort_by_arrival(processes, n);
    reset_processes(processes, n);
    
    int current_time = 0;
    int completed_count = 0;
    
    // 一个简单队列数组，足够囊括所有的执行块插入
    int queue[5000];
    int head = 0, tail = 0;
    bool in_queue[1000] = {false};
    
    // 初始化队列：放入在时间 0 已到达的进程
    if (processes[0].arrival_time > 0) {
        current_time = processes[0].arrival_time;
        printf("[时间  0 - %2d] CPU 空闲\n", current_time);
    }
    
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival_time <= current_time) {
            queue[tail++] = i;
            in_queue[i] = true;
        }
    }
    
    printf("\n=== [RR] 时间片轮转调度执行顺序 (Time Quantum = %d) ===\n", time_quantum);
    
    while (completed_count < n) {
        if (head == tail) { 
            // 队列为空，寻找下一个最近到达的进程跳过空闲时间
            int next_arrival = 99999999;
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > 0 && processes[i].arrival_time > current_time) {
                    if (processes[i].arrival_time < next_arrival) {
                        next_arrival = processes[i].arrival_time;
                    }
                }
            }
            printf("[时间 %2d - %2d] CPU 空闲\n", current_time, next_arrival);
            current_time = next_arrival;
            
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > 0 && processes[i].arrival_time <= current_time && !in_queue[i]) {
                    queue[tail++] = i;
                    in_queue[i] = true;
                }
            }
            continue;
        }
        
        int idx = queue[head++];
        in_queue[idx] = false;
        
        int run_time = (processes[idx].remaining_time > time_quantum) ? time_quantum : processes[idx].remaining_time;
        
        printf("[时间 %2d - %2d] 进程 P%d 正在运行\n", current_time, current_time + run_time, processes[idx].pid);
        
        current_time += run_time;
        processes[idx].remaining_time -= run_time;
        
        // 在该进程执行期间，是否有新进程到达？加入队列
        for (int i = 0; i < n; i++) {
            if (processes[i].remaining_time > 0 && processes[i].arrival_time <= current_time && i != idx && !in_queue[i]) {
                queue[tail++] = i;
                in_queue[i] = true;
            }
        }
        
        // 如果当前进程也没有执行完，重新加入队尾
        if (processes[idx].remaining_time > 0) {
            queue[tail++] = idx;
            in_queue[idx] = true;
        } else {
            // 当前进程执行完毕
            processes[idx].completion_time = current_time;
            processes[idx].turnaround_time = processes[idx].completion_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed_count++;
        }
    }
    print_processes(processes, n);
}
void run_task1_scheduler() {
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
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); 
            break;
        }

        if (choice == 1) {
            fcfs(processes, n);
        } else if (choice == 2) {
            sjf(processes, n);
        } else if (choice == 3) {
            int time_quantum;
            printf("请输入时间片大小(Time Quantum): ");
            if (scanf("%d", &time_quantum) == 1 && time_quantum > 0) {
                rr(processes, n, time_quantum);
            } else {
                printf("时间片无效\n");
                while(getchar() != '\n');
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
