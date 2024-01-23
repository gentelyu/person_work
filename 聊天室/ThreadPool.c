#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Queue.h"
#include "ThreadPool.h"
#include "DoubleLinkList.h"
#include "StdThread.h"
#include "pthread.h"

#define true 1
#define false 0
#define Sleep_time 10 // 管理者线程睡眠时间

// 任务处理有先后，先来的先处理，这里用队列，任务不能无限制增加，得设置最大任务数量,达到最任务数量就不再接受新任务了，也是防止黑客攻击
// 需要一个区间，保证线程的数量,根据业务情况而定

typedef struct ThreadPool
{
    DLlist threads;      // 线程指针存放在双链表
    LQueue task_queue;   // 将任务存在队列，先来先处理
    int min_thrd_num;    // 最少线程数
    int max_thrd_num;    // 最大线程数
    int max_queue_size;  // 任务队列的长度
    int busy_thrd_num;   // 正在忙的线程数量，用来判断有多少个线程在忙
    int exit_thread_num; // 需要退出的线程数量

    pthread_mutex_t pool_mutex;      // 线程池的锁
    pthread_mutex_t busy_thrd_mutex; // 计数器的锁
    pthread_cond_t queue_not_empty;  // 任务队列有任务的条件，条件变量,任务队列不为空的时候激发条件变量
    pthread_cond_t queue_not_full;   // 任务队列不满的条件

    Thread *admin_thread; // 管理者线程

    int shutdown; // 标志位，用来判断让线程停下来

} ThreadP;

typedef struct Task // 任务结构体
{
    void *(*function)(void *,void *); // 线程函数
    void *arg1;//参数1
    void *arg2;//参数2
} task;

task *CreateTask(void *(*func)(void *,void *), void *arg1,void *arg2) // 创建任务
{
    task *t = (task *)malloc(sizeof(task));
    if (t == NULL)
    {
        printf("task malloc error!\n");
        return NULL;
    }

    t->function = func;
    t->arg1 = arg1;
    t->arg2 = arg2;
    return t;
}

void FreeTask(task *t) // 放任务
{
    free(t);
}

void *thread_worker(void *arg) // 线程任务函数,放在创造线程函数里
{
    ThreadP *p = (ThreadP *)arg; // 得解决线程同步问题

    while (1) // 做完任务后再去抢锁，一直循环，这就是线程池
    {
        pthread_mutex_lock(&p->pool_mutex);                              // 给线程上动态锁
        while (IsQEmpty(&p->task_queue) == true && p->shutdown == false) // 抢到锁得看看有没有任务，没任务就等待，这里用条件变量,加条件用于决定运行还是停止
        {
            pthread_cond_wait(&p->queue_not_empty, &p->pool_mutex); // 理解一下，这里等待哪里的信号

            if (p->exit_thread_num > 0) // 如果有裁员名单
            {
                p->exit_thread_num--; // 减减裁员数量

                // 把裁员进程的所在链表删掉
                struct Node *TravelPoint = p->threads.head;
                while (TravelPoint != NULL)
                {
                    Thread *t = (Thread *)TravelPoint->data;
                    if (GetThreadID(t) == pthread_self())//pthread_self拿到当前进程的ID,然后在链表里找到他把他杀了
                    {
                        DBRemoveByElement(&p->threads, t);
                        break;
                    }
                    TravelPoint = TravelPoint->next;
                }
                pthread_mutex_unlock(&p->pool_mutex);
                pthread_exit(NULL);
            }
        }

        if (p->shutdown == true) // 如果上面停止下面就不要执行了
        {
            pthread_mutex_unlock(&p->pool_mutex); // 解锁
            pthread_exit(NULL);                   // 退线程
        }
        task *tk = (task *)(*Pop(&p->task_queue)); // 弹出来的是void**二级指针,先取值，再强转

        pthread_mutex_unlock(&p->pool_mutex); // 解锁放掉的时机是 拿到之后放掉

        pthread_cond_broadcast(&p->queue_not_full); // 发信号给等待---**,让新任务进来

        pthread_mutex_lock(&p->busy_thrd_mutex);//计数器锁 开1
        p->busy_thrd_num++;
        pthread_mutex_unlock(&p->busy_thrd_mutex);//计数器锁 关1

        tk->function(tk->arg1,tk->arg2); // 调用这个函数，传入这个参数，就是执行这个函数

        pthread_mutex_lock(&p->busy_thrd_mutex);//计数器锁 开2
        p->busy_thrd_num--;//线程执行完回电函数
        pthread_mutex_unlock(&p->busy_thrd_mutex);//计数器锁 关2

        FreeTask(tk); // 任务做完，放掉
    }
}

void *thread_manager(void *arg) // 管理函数
{
    ThreadP *p = (ThreadP *)arg;
    while (p->shutdown != true) // 给个判断条件，在我想停下的时候他就停下来
    {
        sleep(Sleep_time);                  // 每隔一段时间检查一次,这个时间可以设个接口把权限交给别人
        pthread_mutex_lock(&p->pool_mutex); // 加锁1
        // 检查工作线程状态，如果外面等待任务数量的超过了工作线程的数量或者空闲的线程太多

        int queueLen = GetQueueLen(&p->task_queue); //
        int thread_num = GetDBlistLen(&p->threads); //

        if (p->busy_thrd_num < queueLen && thread_num < p->max_thrd_num)
        // 不能无限制加，不超过最大线程数
        {
            int add = (queueLen / 2) > (p->max_thrd_num -thread_num) ? (p->max_thrd_num - thread_num) : (queueLen / 2);
            for (int i = 0; i < add; i++)
            {
                Thread *t = InitThread(thread_worker, p);
                DBInsertTail(&p->threads, t);
            }
        }

        thread_num = GetDBlistLen(&p->threads);
        // destory自杀
        if (thread_num > p->busy_thrd_num * 2 && thread_num > p->min_thrd_num) // 总线程的数量大于忙活线程数量的2倍
        {
            int minus_thrd_num = (thread_num - p->busy_thrd_num) / 2; //空闲线程一半的数量
            int minus = minus_thrd_num > (thread_num - p->min_thrd_num) ? (thread_num - p->min_thrd_num) : minus_thrd_num; // 裁员名额

            p->exit_thread_num = minus; // 给要减的线程数量赋值
            pthread_mutex_unlock(&p->pool_mutex);
            for (int i = 0; i < minus; i++)
            {
                pthread_cond_broadcast(&p->queue_not_empty); // 唤醒正在等待的线程,让他们自杀
            }
            continue;//重新进入while循环，看是否还需调整
        }
        pthread_mutex_unlock(&p->pool_mutex); // 解锁1
    }
    pthread_exit(NULL);
}

ThreadP *InitThreadPool(int max_thrd_num, int min_thrd_num, int max_queue_size)
{
    ThreadP *p = (ThreadP *)malloc(sizeof(ThreadP));
    if (p == NULL)
    {
        printf("InitThreadPool malloc error!\n");
        return NULL;
    }

    InitDLlist(&p->threads);
    InitQueue(&p->task_queue);

    p->max_queue_size = max_queue_size;
    p->min_thrd_num = min_thrd_num;
    p->max_thrd_num = max_thrd_num;
    p->busy_thrd_num = 0;
    p->exit_thread_num = 0;

    pthread_mutex_init(&p->pool_mutex, NULL); // 线程 访问属性为NULL，视为默认属性
    pthread_mutex_init(&p->busy_thrd_mutex, NULL);
    pthread_cond_init(&p->queue_not_empty, NULL); 
    pthread_cond_init(&p->queue_not_full, NULL);  

    p->shutdown = false;

    for (int i = 0; i < p->max_thrd_num; i++) // 创建的数量先按照最大的容量进行创建
    {
        Thread *t = InitThread(thread_worker, p); // 传整个结构体指针，以后让他自己判断 ，没任务了就自杀
        DBInsertTail(&p->threads, t);             // 把线程放进链表
    }

    p->admin_thread = InitThread(thread_manager, p); // 管理者线程

    return p;
}

void Threadp_AddTask(ThreadP *p, void *(*func)(void *,void *), void *arg1,void *arg2) // 加任务
{
    pthread_mutex_lock(&p->pool_mutex);                      // 操作队列，公共资源，先加锁
    while (GetQueueLen(&p->task_queue) == p->max_queue_size) // 判断任务队列有没有满
    {
        pthread_cond_wait(&p->queue_not_full, &p->pool_mutex); // 等待不满的时候  **
    }

    if (p->shutdown == true) // 判断还要不要运行
    {
        pthread_mutex_unlock(&p->pool_mutex); // 直接解锁
        return;
    }

    QPush(&p->task_queue, CreateTask(func, arg1,arg2)); // 任务入队列
    pthread_cond_broadcast(&p->queue_not_empty);  // 解锁之前给线程发信号来执行
    pthread_mutex_unlock(&p->pool_mutex);         // 解锁
}

void DestoryThreadPool(ThreadP *p)
{
    if (p == NULL)
    {
        return;
    }

    p->shutdown = true; // 杀管理者线程

    JoinThread(p->admin_thread);//等待线程结束
    free(p->admin_thread); // 释放管理员线程

    // 杀等待线程和工作线程
    int len = GetDBlistLen(&p->threads);
    for (int i = 0; i < len; i++) // 在干活的不会自杀，他们在
    {
        pthread_cond_broadcast(&p->queue_not_empty);
    }

    struct Node *travelPoint = p->threads.head;
    while (travelPoint != NULL)
    {
        Thread *t = (Thread *)travelPoint->data;
        JoinThread(t);
        free(t);
        travelPoint = travelPoint->next;
    }
    FreeDLlist(&p->threads);

    // 杀任务队列
    while (IsQEmpty(&p->task_queue) != true)
    {
        task *t = (task *)(*Pop(&p->task_queue));
        free(t);
    }
    FreeQueue(&p->task_queue);

    pthread_mutex_destroy(&p->pool_mutex);
    pthread_mutex_destroy(&p->busy_thrd_mutex);

    pthread_cond_destroy(&p->queue_not_empty);
    pthread_cond_destroy(&p->queue_not_full);
}
