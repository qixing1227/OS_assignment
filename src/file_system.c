#include "file_system.h"
#include <string.h>

// ============================================
// 4. 文件系统模拟 (File System)
// ============================================

#define DISK_BLOCKS 256     // 虚拟磁盘总块数
#define BLOCK_SIZE 64       // 每个数据块大小(字节)
#define MAX_FILES 50        // 根目录最大文件数

// FAT表标志位宏
#define FAT_FREE -1         // 空闲块
#define FAT_EOF  -2         // 文件结束(End of File)

// 文件控制块(FCB)
typedef struct {
    char filename[32];      // 文件名
    int size;               // 文件实际大小(字节)
    int start_block;        // 起始逻辑块号
    bool is_used;           // 该目录项是否已被占用
} FCB;

// --- 全局文件系统数据结构 ---
// 1. FAT 表 (既用来链接文件块，又用于空闲空间管理)
int fat[DISK_BLOCKS];

// 2. 根目录区
FCB root_dir[MAX_FILES];

// 3. 虚拟磁盘数据区
char virtual_disk[DISK_BLOCKS][BLOCK_SIZE];

// 是否已经初始化格式化
bool fs_initialized = false;

// --------------------------------------------
// 内部辅助函数
// --------------------------------------------

void init_fs() {
    // 初始化 FAT 表为空闲
    for (int i = 0; i < DISK_BLOCKS; i++) {
        fat[i] = FAT_FREE;
    }
    // 初始化根目录
    for (int i = 0; i < MAX_FILES; i++) {
        root_dir[i].is_used = false;
    }
    // 清空磁盘数据
    memset(virtual_disk, 0, sizeof(virtual_disk));
    
    fs_initialized = true;
    printf("[成功] 虚拟文件系统初始化/格式化完成！\n");
    printf("磁盘参数: 总块数 %d, 块大小 %d 字节, 总容量 %d 字节\n", DISK_BLOCKS, BLOCK_SIZE, DISK_BLOCKS * BLOCK_SIZE);
}

// 统计空闲块数目
int get_free_blocks_count() {
    int count = 0;
    for (int i = 0; i < DISK_BLOCKS; i++) {
        if (fat[i] == FAT_FREE) count++;
    }
    return count;
}

// 查找文件FCB索引
int find_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_dir[i].is_used && strcmp(root_dir[i].filename, filename) == 0) {
            return i;
        }
    }
    return -1; // 没找到
}

// --------------------------------------------
// 核心操作接口
// --------------------------------------------

void fs_create_file() {
    char name[32];
    int size;
    printf("请输入新建文件名: ");
    scanf("%s", name);
    
    if (find_file(name) != -1) {
        printf("[失败] 文件 '%s' 已存在！\n", name);
        return;
    }
    
    printf("请输入预分配的文件大小(字节): ");
    scanf("%d", &size);
    if (size <= 0) {
        printf("[错误] 文件大小必须大于0！\n");
        return;
    }
    
    int needed_blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (get_free_blocks_count() < needed_blocks) {
        printf("[失败] 磁盘空闲空间不足，需要 %d 块，剩余 %d 块。\n", needed_blocks, get_free_blocks_count());
        return;
    }
    
    // 找一个空的目录项
    int dir_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!root_dir[i].is_used) {
            dir_index = i;
            break;
        }
    }
    if (dir_index == -1) {
        printf("[失败] 根目录已满，无法创建更多文件。\n");
        return;
    }
    
    // 分配物理块并建立FAT链表
    int start_block = -1;
    int prev_block = -1;
    int allocated = 0;
    
    for (int i = 0; i < DISK_BLOCKS && allocated < needed_blocks; i++) {
        if (fat[i] == FAT_FREE) {
            if (start_block == -1) {
                start_block = i;
            } else {
                fat[prev_block] = i; // 链接上一块
            }
            fat[i] = FAT_EOF; // 暂时标记为文件尾
            prev_block = i;
            allocated++;
        }
    }
    
    // 注册到目录
    strcpy(root_dir[dir_index].filename, name);
    root_dir[dir_index].size = size;
    root_dir[dir_index].start_block = start_block;
    root_dir[dir_index].is_used = true;
    
    printf("[成功] 文件 '%s' 创建成功！占用 %d 个物理块(起始块号: %d)。\n", name, needed_blocks, start_block);
}

void fs_delete_file() {
    char name[32];
    printf("请输入要删除的文件名: ");
    scanf("%s", name);
    
    int dir_index = find_file(name);
    if (dir_index == -1) {
        printf("[失败] 文件 '%s' 不存在！\n", name);
        return;
    }
    
    // 释放FAT表中的物理块
    int curr_block = root_dir[dir_index].start_block;
    while (curr_block != FAT_EOF && curr_block >= 0 && curr_block < DISK_BLOCKS) {
        int next_block = fat[curr_block];
        fat[curr_block] = FAT_FREE; // 标记为空闲
        memset(virtual_disk[curr_block], 0, BLOCK_SIZE); // 清洗磁盘数据(可选)
        curr_block = next_block;
    }
    
    // 删除目录项
    root_dir[dir_index].is_used = false;
    printf("[成功] 文件 '%s' 及其对应存储空间已成功删除回收。\n", name);
}

void fs_write_file() {
    char name[32];
    printf("请输入要写入的文件名: ");
    scanf("%s", name);
    
    int dir_index = find_file(name);
    if (dir_index == -1) {
        printf("[失败] 文件 '%s' 不存在！\n", name);
        return;
    }
    
    // 简单起见，按行读取输入作为写入内容
    char content[1024];
    printf("请输入要写入的文件内容(不支持空格，遇到回车结束): \n> ");
    scanf("%s", content);
    
    int write_len = strlen(content);
    if (write_len > root_dir[dir_index].size) {
        printf("[警告] 写入内容长度(%d)超出了文件的最大容量(%d)！将发生截断写入。\n", write_len, root_dir[dir_index].size);
        write_len = root_dir[dir_index].size;
    }
    
    int curr_block = root_dir[dir_index].start_block;
    int offset = 0; // content中的写入偏移位置
    
    while (curr_block != FAT_EOF && offset < write_len) {
        // 当前块能写入多少数据？
        int to_write = (write_len - offset < BLOCK_SIZE) ? (write_len - offset) : BLOCK_SIZE;
        
        // 复制数据到虚拟磁盘缓冲
        strncpy(virtual_disk[curr_block], content + offset, to_write);
        
        offset += to_write;
        curr_block = fat[curr_block]; // 顺着 FAT 找下一块
    }
    printf("[成功] 文件 '%s' 写入完成，共写入了 %d 字节。\n", name, write_len);
}

void fs_read_file() {
    char name[32];
    printf("请输入要读取的文件名: ");
    scanf("%s", name);
    
    int dir_index = find_file(name);
    if (dir_index == -1) {
        printf("[失败] 文件 '%s' 不存在！\n", name);
        return;
    }
    
    printf("\n--- 文件 '%s' 内容 ---\n", name);
    int curr_block = root_dir[dir_index].start_block;
    int bytes_read = 0;
    int target_size = root_dir[dir_index].size;
    
    while (curr_block != FAT_EOF && curr_block >= 0 && bytes_read < target_size) {
        char buf[BLOCK_SIZE + 1];
        memset(buf, 0, BLOCK_SIZE + 1);
        
        int to_read = (target_size - bytes_read < BLOCK_SIZE) ? (target_size - bytes_read) : BLOCK_SIZE;
        strncpy(buf, virtual_disk[curr_block], to_read);
        
        printf("%s", buf);
        
        bytes_read += to_read;
        curr_block = fat[curr_block];
    }
    printf("\n----------------------\n");
}

void fs_list_dir() {
    printf("\n=== 根目录 (Root Directory) ===\n");
    printf("%-15s %-10s %-12s\n", "文件名", "大小(B)", "起始逻辑块");
    int count = 0;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_dir[i].is_used) {
            printf("%-15s %-10d %-12d\n", root_dir[i].filename, root_dir[i].size, root_dir[i].start_block);
            count++;
        }
    }
    if (count == 0) {
        printf("(空目录)\n");
    }
    
    // 显示空闲空间
    int free_blocks = get_free_blocks_count();
    printf("\n[磁盘信息] 空闲块数: %d/%d (%.2f%%) | 剩余空间: %d 字节\n", 
            free_blocks, DISK_BLOCKS, (float)free_blocks / DISK_BLOCKS * 100.0, free_blocks * BLOCK_SIZE);
}


// ============================================
// 统一菜单入口
// ============================================
void run_task4_file_system() {
    if (!fs_initialized) {
        init_fs();
    }
    
    while(1) {
        printf("\n====== 任务四: 文件系统管理模块 ======\n");
        printf("1. 创建文件 (Create File)\n");
        printf("2. 删除文件 (Delete File & Free Space)\n");
        printf("3. 写入文件 (Write File)\n");
        printf("4. 读取文件 (Read File)\n");
        printf("5. 列出目录与磁盘空间 (List Directory & Space)\n");
        printf("6. 重新格式化磁盘 (Format Disk)\n");
        printf("0. 返回主菜单\n");
        printf("请输入选择> ");
        
        int choice;
        if (scanf("%d", &choice) != 1) break;
        
        if (choice == 1) {
            fs_create_file();
        } else if (choice == 2) {
            fs_delete_file();
        } else if (choice == 3) {
            fs_write_file();
        } else if (choice == 4) {
            fs_read_file();
        } else if (choice == 5) {
            fs_list_dir();
        } else if (choice == 6) {
            char confirm;
            printf("警告：格式化将清空所有数据！确定吗？(y/n): ");
            scanf(" %c", &confirm);
            if (confirm == 'y' || confirm == 'Y') init_fs();
            else printf("已取消格式化。\n");
        } else if (choice == 0) {
            break;
        } else {
            printf("无效选择，请重新输入。\n");
        }
    }
}