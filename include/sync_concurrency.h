#ifndef SYNC_CONCURRENCY_H
#define SYNC_CONCURRENCY_H

#include "common.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// 生产者-消费者问题实验
void run_producer_consumer();

// 读者-写者问题实验
void run_reader_writer();

// 哲学家进餐问题实验
void run_dining_philosophers();

// 任务三：进程同步与并发控制主菜单入口
void run_task3_sync_concurrency();

#endif
