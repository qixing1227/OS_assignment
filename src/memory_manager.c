#include "memory_manager.h"

// ============================================
// 1. 页面置换算法部分
// ============================================

void print_frames(int frames[], int capacity) {
    for (int i = 0; i < capacity; i++) {
        if (frames[i] == -1) {
            printf("[ ] ");
        } else {
            printf("[%d] ", frames[i]);
        }
    }
}

// 查找页面是否已经在内存中
bool is_page_hit(int frames[], int capacity, int page) {
    for (int i = 0; i < capacity; i++) {
        if (frames[i] == page) {
            return true;
        }
    }
    return false;
}

void page_replacement_fifo(int pages[], int n, int capacity) {
    int frames[100]; // 假设最多100个物理块
    for (int i = 0; i < capacity; i++) frames[i] = -1;
    
    int page_faults = 0;
    int index = 0; // FIFO淘汰指针
    
    printf("\n=== [FIFO] 页面置换过程 ===\n");
    for (int i = 0; i < n; i++) {
        printf("访问页面 %2d: ", pages[i]);
        if (is_page_hit(frames, capacity, pages[i])) {
            printf(" 命中(Hit)   | 物理块: ");
            print_frames(frames, capacity);
            printf("\n");
        } else {
            frames[index] = pages[i];
            index = (index + 1) % capacity;
            page_faults++;
            printf("缺页(Fault) | 物理块: ");
            print_frames(frames, capacity);
            printf("\n");
        }
    }
    printf("------------------------------\n");
    printf("总访问次数 : %d\n", n);
    printf("总缺页次数 : %d\n", page_faults);
    printf("缺页率     : %.2f%%\n", (float)page_faults / n * 100);
}

void page_replacement_lru(int pages[], int n, int capacity) {
    int frames[100]; 
    int last_used[100]; // 记录各个物理块最后访问的时间
    for (int i = 0; i < capacity; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }
    
    int page_faults = 0;
    int current_time = 0; // 虚拟时间，用于记录最后使用
    
    printf("\n=== [LRU] 页面置换过程 ===\n");
    for (int i = 0; i < n; i++) {
        current_time++;
        printf("访问页面 %2d: ", pages[i]);
        
        // 1. 判断是否命中
        bool hit = false;
        for (int j = 0; j < capacity; j++) {
            if (frames[j] == pages[i]) {
                hit = true;
                last_used[j] = current_time; // 更新最后访问时间
                break;
            }
        }
        
        if (hit) {
            printf(" 命中(Hit)   | 物理块: ");
            print_frames(frames, capacity);
            printf("\n");
        } else {
            // 2. 缺页，需要置换 (找一个空闲，或者找最久未使用的)
            int replace_index = -1;
            int min_time = 99999999;
            for (int j = 0; j < capacity; j++) {
                if (frames[j] == -1) {
                    replace_index = j;
                    break;
                }
                if (last_used[j] < min_time) {
                    min_time = last_used[j];
                    replace_index = j;
                }
            }
            
            frames[replace_index] = pages[i];
            last_used[replace_index] = current_time;
            page_faults++;
            
            printf("缺页(Fault) | 物理块: ");
            print_frames(frames, capacity);
            printf("\n");
        }
    }
    printf("------------------------------\n");
    printf("总访问次数 : %d\n", n);
    printf("总缺页次数 : %d\n", page_faults);
    printf("缺页率     : %.2f%%\n", (float)page_faults / n * 100);
}

void run_page_replacement() {
    int capacity;
    printf("\n[页面置换算法测试]\n");
    printf("请输入物理区块数量 (如 3 或 4): ");
    scanf("%d", &capacity);
    
    int n;
    printf("请输入页面访问序列长度 (如 12): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("输入无效！\n");
        while(getchar() != '\n');
        return;
    }
    
    int *pages = (int*)malloc(n * sizeof(int));
    if (!pages) return;
    
    printf("请输入页面访问序列 (用空格隔开，如 4 3 2 1 4 3 5 4 3 2 1 5):\n> ");
    for (int i = 0; i < n; i++) {
        scanf("%d", &pages[i]);
    }
    
    int choice;
    printf("\n请选择一种页面置换算法:\n");
    printf("1. FIFO (先进先出)\n");
    printf("2. LRU  (最近最久未使用)\n");
    printf("> ");
    scanf("%d", &choice);
    
    if (choice == 1) page_replacement_fifo(pages, n, capacity);
    else if (choice == 2) page_replacement_lru(pages, n, capacity);
    else printf("无效选择！\n");
    
    free(pages);
}

// ============================================
// 2. 动态分区分配算法部分
// ============================================

MemoryBlock memory_pool[MAX_BLOCKS];
int block_count = 0;
int next_block_id = 0;

void init_memory(int total_size) {
    block_count = 1;
    memory_pool[0].id = next_block_id++;
    memory_pool[0].start_addr = 0;
    memory_pool[0].size = total_size;
    memory_pool[0].is_free = true;
    memory_pool[0].process_id = -1;
    printf("[系统提示] 内存已初始化完成，总容量: %d\n", total_size);
}

void print_memory_status() {
    printf("\n------ 内存当前状态 ------\n");
    printf("%-5s %-12s %-10s %-10s %-8s\n", "ID", "起始地址", "大小", "状态", "进程ID");
    for (int i = 0; i < block_count; i++) {
        printf("%-5d 0x%08X %-10d %-10s %-8d\n",
            memory_pool[i].id,
            memory_pool[i].start_addr,
            memory_pool[i].size,
            memory_pool[i].is_free ? "[空闲]" : "[占用]",
            memory_pool[i].is_free ? -1 : memory_pool[i].process_id);
    }
    printf("--------------------------\n\n");
}

// 首次适应算法 (FF)
bool allocate_memory_ff(int process_id, int size) {
    for (int i = 0; i < block_count; i++) {
        if (memory_pool[i].is_free && memory_pool[i].size >= size) {
            // 如果剩余大小足以分裂出一个新块 (碎片管理，如剩余大于0)
            if (memory_pool[i].size > size) {
                // 向后滑动数组给新分割的块腾个位（数组模拟链表插入）
                for (int j = block_count; j > i + 1; j--) {
                    memory_pool[j] = memory_pool[j-1];
                }
                block_count++;
                
                // 新空闲块
                memory_pool[i+1].id = next_block_id++;
                memory_pool[i+1].start_addr = memory_pool[i].start_addr + size;
                memory_pool[i+1].size = memory_pool[i].size - size;
                memory_pool[i+1].is_free = true;
                memory_pool[i+1].process_id = -1;
            }
            
            // 当前块被占用
            memory_pool[i].size = size;
            memory_pool[i].is_free = false;
            memory_pool[i].process_id = process_id;
            
            printf("[成功] 首次适应(FF): 分配了进程 P%d，大小为 %d\n", process_id, size);
            return true;
        }
    }
    printf("[失败] 首次适应(FF): 内存不足，进程 P%d (大小 %d) 无法分配\n", process_id, size);
    return false;
}

// 最佳适应算法 (BF)
bool allocate_memory_bf(int process_id, int size) {
    int best_index = -1;
    int min_frag = 99999999;
    
    for (int i = 0; i < block_count; i++) {
        if (memory_pool[i].is_free && memory_pool[i].size >= size) {
            if (memory_pool[i].size - size < min_frag) {
                min_frag = memory_pool[i].size - size;
                best_index = i;
            }
        }
    }
    
    if (best_index == -1) {
        printf("[失败] 最佳适应(BF): 内存不足，进程 P%d (大小 %d) 无法分配\n", process_id, size);
        return false;
    }
    
    int i = best_index;
    if (memory_pool[i].size > size) {
        for (int j = block_count; j > i + 1; j--) {
            memory_pool[j] = memory_pool[j-1];
        }
        block_count++;
        
        memory_pool[i+1].id = next_block_id++;
        memory_pool[i+1].start_addr = memory_pool[i].start_addr + size;
        memory_pool[i+1].size = memory_pool[i].size - size;
        memory_pool[i+1].is_free = true;
        memory_pool[i+1].process_id = -1;
    }
    
    memory_pool[i].size = size;
    memory_pool[i].is_free = false;
    memory_pool[i].process_id = process_id;
    
    printf("[成功] 最佳适应(BF): 分配了进程 P%d，大小为 %d\n", process_id, size);
    return true;
}

// 释放内存并合并相邻的空闲块
void free_memory(int process_id) {
    bool found = false;
    for (int i = 0; i < block_count; i++) {
        if (!memory_pool[i].is_free && memory_pool[i].process_id == process_id) {
            memory_pool[i].is_free = true;
            memory_pool[i].process_id = -1;
            found = true;
            printf("[成功] 回收了进程 P%d 所占用的内存 (大小: %d)\n", process_id, memory_pool[i].size);
            break;
        }
    }
    
    if (!found) {
        printf("[失败] 未能找到被 P%d 占用的内存区\n", process_id);
        return;
    }
    
    // 合并相邻空闲块
    for (int i = 0; i < block_count - 1; i++) {
        if (memory_pool[i].is_free && memory_pool[i+1].is_free) {
            // 合并 i 和 i+1
            memory_pool[i].size += memory_pool[i+1].size;
            // 移动后面的填补
            for (int k = i + 1; k < block_count - 1; k++) {
                memory_pool[k] = memory_pool[k+1];
            }
            block_count--;
            i--; // 继续检查当前合并后的块是否和下一个也是空闲的
        }
    }
}

void run_dynamic_partition() {
    int total_limit;
    printf("\n[动态分区分配测试]\n");
    printf("请输入系统总内存大小 (例如 1000): ");
    scanf("%d", &total_limit);
    
    init_memory(total_limit);
    print_memory_status();
    
    int algo_choice;
    printf("请选择内存分配算法:\n1. FF (首次适应)\n2. BF (最佳适应)\n> ");
    scanf("%d", &algo_choice);
    
    while (1) {
        printf("\n操作菜单: [1] 分配进程内存  [2] 回收进程内存  [3] 查看内存状态  [0] 返回上级\n> ");
        int choice;
        scanf("%d", &choice);
        
        if (choice == 1) {
            int pid, size;
            printf("请输入 进程ID 和 需求大小 (用空格隔开): ");
            scanf("%d %d", &pid, &size);
            if (algo_choice == 1) allocate_memory_ff(pid, size);
            else if (algo_choice == 2) allocate_memory_bf(pid, size);
            else printf("算法选择错误/未生效\n");
        } else if (choice == 2) {
            int pid;
            printf("请输入要回收的进程ID: ");
            scanf("%d", &pid);
            free_memory(pid);
        } else if (choice == 3) {
            print_memory_status();
        } else if (choice == 0) {
            break;
        } else {
            printf("无效输入\n");
        }
    }
}

// ============================================
// 统一入口菜单
// ============================================
void run_task2_memory_management() {
    while(1) {
        printf("\n====== 任务二: 内存管理模块 ======\n");
        printf("1. 测试 页面置换算法 (FIFO, LRU)\n");
        printf("2. 测试 动态分区分配与回收 (FF, BF)\n");
        printf("0. 返回主菜单\n");
        printf("请输入选择> ");
        
        int choice;
        if (scanf("%d", &choice) != 1) break;
        
        if (choice == 1) {
            run_page_replacement();
        } else if (choice == 2) {
            run_dynamic_partition();
        } else if (choice == 0) {
            break;
        } else {
            printf("无效的选择，请重新输入。\n");
        }
    }
}