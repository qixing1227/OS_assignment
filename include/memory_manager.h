#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "common.h"

// ============================================
// 1. 页面置换 (Page Replacement)
// ============================================

// FIFO 页面置换算法
void page_replacement_fifo(int pages[], int n, int capacity);

// LRU 页面置换算法
void page_replacement_lru(int pages[], int n, int capacity);

// 页面置换模块交互入口
void run_page_replacement();


// ============================================
// 2. 动态分区分配 (Dynamic Partition Allocation)
// ============================================

// 内存分区块结构体 (采用基于数组的链表模式进行抽象管理)
typedef struct {
    int id;             // 分区编号
    int start_addr;     // 起始地址
    int size;           // 分区大小
    bool is_free;       // 是否空闲
    int process_id;     // 占据该分区的进程ID，-1表示无
} MemoryBlock;

#define MAX_BLOCKS 1000

// 初始化内存池
void init_memory(int total_size);

// 打印当前内存分布状态
void print_memory_status();

// 首次适应算法 (First Fit)
bool allocate_memory_ff(int process_id, int size);

// 最佳适应算法 (Best Fit)
bool allocate_memory_bf(int process_id, int size);

// 释放内存并合并相邻空闲区
void free_memory(int process_id);

// 动态分区模块交互入口
void run_dynamic_partition();

// 统一的内存管理任务入口
void run_task2_memory_management();

#endif
