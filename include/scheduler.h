#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

// 进程控制块结构体
typedef struct {
    int pid;             // 进程ID (通常用数字 1, 2, 3...)
    int arrival_time;    // 到达时间 (Arrival Time)
    int burst_time;      // 执行时间 (Burst Time)
    int priority;        // 优先级 (数字越小优先级越高，或反之，依实现而定)
    
    // 以下为统计与调度时用到的辅助变量
    int remaining_time;  // 剩余执行时间（供时间片轮转等抢占式调度使用）
    int start_time;      // 实际开始运行时间
    int completion_time; // 完成时间
    int turnaround_time; // 周转时间 = 完成时间 - 到达时间
    int waiting_time;    // 等待时间 = 周转时间 - 执行时间
} Process;

// 调度算法接口
void fcfs(Process processes[], int n);                   // 先来先服务算法
void sjf(Process processes[], int n);                    // 短作业优先算法(非抢占式)
void rr(Process processes[], int n, int time_quantum);   // 时间片轮转调度算法

// 辅助与状态展示接口
void print_processes(Process processes[], int n);        
void input_processes(Process **processes, int *n);       
void reset_processes(Process processes[], int n);

#endif