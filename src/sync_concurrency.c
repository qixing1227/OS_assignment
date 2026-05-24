#include "sync_concurrency.h"

// ============================================
// 1. 生产者-消费者问题 (Producer-Consumer)
// ============================================
#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t empty;
sem_t full;
pthread_mutex_t prod_cons_mutex;

void* producer(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 4; i++) { // 每个生产者生产4次
        int item = rand() % 100; // 产生随机数据
        
        sem_wait(&empty); // 等待缓冲区有空位
        pthread_mutex_lock(&prod_cons_mutex); // 互斥访问缓冲区
        
        buffer[in] = item;
        printf("[生产者 %d] 生产数据: %2d, 放入位置: %d\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&prod_cons_mutex);
        sem_post(&full); // 增加已有数据信号量
        
        sleep(1); // 模拟耗时
    }
    return NULL;
}

void* consumer(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 4; i++) { // 每个消费者消费4次
        sem_wait(&full); // 等待缓冲区有数据
        pthread_mutex_lock(&prod_cons_mutex); // 互斥访问缓冲区
        
        int item = buffer[out];
        printf("[消费者 %d] 消费数据: %2d, 取自位置: %d\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        
        pthread_mutex_unlock(&prod_cons_mutex);
        sem_post(&empty); // 增加空位信号量
        
        sleep(2); // 模拟耗时(消费者慢一点)
    }
    return NULL;
}

void run_producer_consumer() {
    printf("\n=== [生产者-消费者问题模拟] ===\n");
    printf("设定: 3个生产者，2个消费者，缓冲区大小 %d\n", BUFFER_SIZE);
    
    // 初始化同步机制
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&prod_cons_mutex, NULL);
    
    pthread_t prods[3], cons[2];
    int p_ids[3] = {1, 2, 3};
    int c_ids[2] = {1, 2};
    
    for(int i = 0; i < 3; i++) pthread_create(&prods[i], NULL, producer, &p_ids[i]);
    for(int i = 0; i < 2; i++) pthread_create(&cons[i], NULL, consumer, &c_ids[i]);
    
    // 等待线程执行完毕
    for(int i = 0; i < 3; i++) pthread_join(prods[i], NULL);
    for(int i = 0; i < 2; i++) pthread_join(cons[i], NULL);
    
    // 销毁同步机制
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&prod_cons_mutex);
    
    printf("--- 生产者/消费者执行完毕 ---\n");
}

// ============================================
// 2. 读者-写者问题 (Reader-Writer, 读者优先策略)
// ============================================
int shared_data = 0;
int read_count = 0;
pthread_mutex_t rw_mutex; // 控制写写互斥、读写互斥的操作
pthread_mutex_t rc_mutex; // 控制 readers_count 的互斥操作

void* writer(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 2; i++) { // 每个写者写两次
        sleep(rand() % 3 + 1); // 随机等待一段时间再写入
        
        pthread_mutex_lock(&rw_mutex);
        // 写操作
        shared_data += 10;
        printf("[写者 %d] 获取写锁，成功写入数据: %d\n", id, shared_data);
        sleep(1); // 模拟写耗时
        pthread_mutex_unlock(&rw_mutex);
    }
    return NULL;
}

void* reader(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 3; i++) { // 每个读者读三次
        sleep(rand() % 2 + 1);
        
        pthread_mutex_lock(&rc_mutex);
        read_count++;
        if (read_count == 1) { // 第一个读者负责加写锁，阻止写者进入
            pthread_mutex_lock(&rw_mutex);
            printf("  -> (第 %d 个读者进入，锁定写锁阻止写者)\n", read_count);
        }
        pthread_mutex_unlock(&rc_mutex);
        
        // 读操作
        printf("[读者 %d] 正在读取数据: %d (当前并发读者数: %d)\n", id, shared_data, read_count);
        sleep(1); // 模拟读耗时，此时可以有其他读者进入
        
        pthread_mutex_lock(&rc_mutex);
        read_count--;
        if (read_count == 0) { // 最后一个读者退出时，释放写锁，允许写者进入
            printf("  -> (所有读者已离开，释放写锁)\n");
            pthread_mutex_unlock(&rw_mutex);
        }
        pthread_mutex_unlock(&rc_mutex);
    }
    return NULL;
}

void run_reader_writer() {
    printf("\n=== [读者-写者问题模拟 (读者优先)] ===\n");
    printf("设定: 3个读者，2个写者\n");
    
    shared_data = 0;
    read_count = 0;
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&rc_mutex, NULL);
    
    pthread_t r_threads[3], w_threads[2];
    int r_ids[3] = {1, 2, 3};
    int w_ids[2] = {1, 2};
    
    for(int i = 0; i < 3; i++) pthread_create(&r_threads[i], NULL, reader, &r_ids[i]);
    for(int i = 0; i < 2; i++) pthread_create(&w_threads[i], NULL, writer, &w_ids[i]);
    
    for(int i = 0; i < 3; i++) pthread_join(r_threads[i], NULL);
    for(int i = 0; i < 2; i++) pthread_join(w_threads[i], NULL);
    
    pthread_mutex_destroy(&rw_mutex);
    pthread_mutex_destroy(&rc_mutex);
    
    printf("--- 读者/写者执行完毕 ---\n");
}

// ============================================
// 3. 哲学家进餐问题 (Dining Philosophers)
// ============================================
#define PHILOSOPHER_COUNT 5
pthread_mutex_t forks[PHILOSOPHER_COUNT];

void* philosopher(void* arg) {
    int id = *((int*)arg);
    int left = id;
    int right = (id + 1) % PHILOSOPHER_COUNT;
    
    for (int i = 0; i < 2; i++) { // 每人进餐两次
        printf("[哲学家 %d] 思考中...\n", id);
        sleep(rand() % 3 + 1);
        
        printf("[哲学家 %d] 肚子饿了，试图拿叉子...\n", id);
        
        // 核心机制：为避免死锁，打破循环等待。
        // 让最后一名哲学家先拿右边的叉子再拿左边，其他人先左后右
        if (id == PHILOSOPHER_COUNT - 1) {
            pthread_mutex_lock(&forks[right]);
            pthread_mutex_lock(&forks[left]);
        } else {
            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);
        }
        
        printf("[哲学家 %d] 拿到两把叉子(%d, %d)，正在进餐(Eating)...\n", id, left, right);
        sleep(2); // 进餐
        
        // 放下叉子
        pthread_mutex_unlock(&forks[left]);
        pthread_mutex_unlock(&forks[right]);
        printf("[哲学家 %d] 吃饱了，放下叉子.\n", id);
    }
    return NULL;
}

void run_dining_philosophers() {
    printf("\n=== [哲学家进餐问题模拟] ===\n");
    printf("设定: 5个哲学家，5把叉子。利用编号打破循环依赖死锁策略\n");
    
    pthread_t phils[PHILOSOPHER_COUNT];
    int p_ids[PHILOSOPHER_COUNT];
    
    // 初始化锁
    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }
    
    // 创建哲学家线程
    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        p_ids[i] = i;
        pthread_create(&phils[i], NULL, philosopher, &p_ids[i]);
    }
    
    // 等待结束
    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        pthread_join(phils[i], NULL);
    }
    
    // 销毁锁
    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        pthread_mutex_destroy(&forks[i]);
    }
    printf("--- 所有哲学家已结束进餐 ---\n");
}


// ============================================
// 统一菜单入口
// ============================================
void run_task3_sync_concurrency() {
    while(1) {
        printf("\n====== 任务三: 进程同步与并发控制 ======\n");
        printf("1. 模拟 生产者-消费者问题\n");
        printf("2. 模拟 读者-写者问题\n");
        printf("3. 模拟 哲学家进餐问题\n");
        printf("0. 返回主菜单\n");
        printf("请输入选择> ");
        
        int choice;
        if (scanf("%d", &choice) != 1) break;
        
        if (choice == 1) {
            run_producer_consumer();
        } else if (choice == 2) {
            run_reader_writer();
        } else if (choice == 3) {
            run_dining_philosophers();
        } else if (choice == 0) {
            break;
        } else {
            printf("无效选择，请重新输入。\n");
        }
    }
}