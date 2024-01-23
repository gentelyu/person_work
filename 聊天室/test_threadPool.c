#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "DoubleLinkList.h"
#include "Queue.h"
#include "StdThread.h"
#include "test_threadPool.h"



#define true 1
#define false 0
#define thread_manager_sleep_time 5

typedef struct threadPool
{
    int max_thread_num;
    int min_thread_num;
    int max_queue_size; //任务队列的最大长度

    
    int busy_thread_num;    //正在工作的线程数量
    int kill_thread_num;    //要杀死的线程数量

    LQueue task_queue;  //任务队列   
    DLlist DL_thread;      //放线程指针的双链表

    pthread_mutex_t pool_mutex; 
    pthread_mutex_t busy_thread_mutex;   
    pthread_cond_t queue_not_empty;  
    pthread_cond_t queue_not_full;   

    Thread *admin_thread;   //管理者线程
    bool shutDown;          //控制线程池工作的标志位

} ThreadP;

/* 任务结构物 */
typedef struct Task
{
    void *(*function)(void *,void *);
    void *arg1;
    void *arg2;
}task;

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

void *thread_woker(void *arg)   //工作者线程的处理函数
{
    ThreadP *p = (ThreadP*)(arg);

    while(1)
    {
        pthread_mutex_lock(&p->pool_mutex); //做任务上锁
        while(p->shutDown == false && IsQEmpty(&p->task_queue) == true)
        {
            pthread_cond_wait(&p->queue_not_empty,&p->pool_mutex);//阻塞，等待任务队列来任务
            if(p->kill_thread_num > 0)
            {
                p->kill_thread_num--;

                struct Node * travelPoint = p->DL_thread.head;

                while(travelPoint != NULL)
                {
                    Thread *t = (Thread*)(travelPoint->data);
                    if(GetThreadID(p) == pthread_self())    //如果找到当前线程，就自杀
                    {
                        DBRemoveByElement(&p->DL_thread,t);
                        break;
                    }
                    travelPoint = travelPoint->next;
                }
                pthread_mutex_unlock(&p->pool_mutex);
                pthread_exit(NULL);
            }
        }

        if(p->shutDown == true)
        {
            pthread_mutex_unlock(&p->pool_mutex); 
            pthread_exit(NULL);                   
        }

        task * tk = (task*)Pop(&p->task_queue);
        pthread_mutex_unlock(&p->pool_mutex);
        pthread_cond_broadcast(&p->queue_not_full);

        /* 正在做任务的线程数量加一 */
        pthread_mutex_lock(&p->busy_thread_mutex);
        p->busy_thread_num++;                           
        pthread_mutex_unlock(&p->busy_thread_mutex);

        tk->function(tk->arg1,tk->arg2);    //做任务

        /* 正在做任务的线程数量减一 */
        pthread_mutex_lock(&p->busy_thread_mutex);
        p->busy_thread_num--;
        pthread_mutex_unlock(&p->busy_thread_mutex);

        free(tk);    
    }
}

void * thread_manager(void *arg)
{
    ThreadP * p = (ThreadP*)(arg);
    while(p->shutDown == false)
    {
        sleep(thread_manager_sleep_time);
        pthread_mutex_lock(&p->pool_mutex);
        
        int queueLen = GetQueueLen(&p->task_queue);   //当前任务队列的长度
        int thread_num = GetDBlistLen(&p->DL_thread); //当前所有的线程数量

        /* 如果需要加线程*/
        if (p->busy_thread_num < queueLen && thread_num < p->max_thread_num)
        {
            int add = (queueLen / 2) > (p->max_thread_num -thread_num) ? (p->max_thread_num - thread_num) : (queueLen / 2);
            for (int i = 0; i < add; i++)
            {
                Thread *t = InitThread(thread_woker, p);
                DBInsertTail(&p->DL_thread, t);
            }
        }

        thread_num = GetDBlistLen(&p->DL_thread);

        /* 看是否需要杀线程 */
        if (thread_num > p->busy_thread_num * 2 && thread_num > p->min_thread_num) 
        {
            int minus_thrd_num = (thread_num - p->busy_thread_num) / 2; 
            int minus = minus_thrd_num > (thread_num - p->min_thread_num) ? (thread_num - p->min_thread_num) : minus_thrd_num; // 裁员名额
            p->kill_thread_num = minus; // 给要减的线程数量赋值

            pthread_mutex_unlock(&p->pool_mutex);
            
            for (int i = 0; i < minus; i++)
            {
                pthread_cond_broadcast(&p->queue_not_empty); // 唤醒正在等待的线程,让他们自杀
            }

            continue;//重新进while循环看是否还需调整     
        }
        pthread_mutex_unlock(&p->pool_mutex);
    }
    pthread_exit(NULL);
}

ThreadP *InitThreadPool(int max_thread_num, int min_thread_num, int max_queue_size)
{
    ThreadP *p = (Thread*)malloc(sizeof(ThreadP));
    if(p == NULL)
    {
        perror("init threadPool error!\n");
        return false;
    }

    InitDLlist(&p->DL_thread);     //初始化存放线程指针的双链表
    InitQueue(&p->task_queue);  //初始化任务队列

    p->max_thread_num = max_thread_num;
    p->min_thread_num = min_thread_num;
    p->max_queue_size = max_queue_size;

    p->kill_thread_num = 0;
    p->busy_thread_num = 0;

    p->shutDown = false;

    pthread_mutex_init(&p->pool_mutex,NULL);        //NULL为默认属性 
    pthread_mutex_init(&p->busy_thread_mutex,NULL);
    pthread_cond_init(&p->queue_not_empty,NULL);    //NULL也为默认属性
    pthread_cond_init(&p->queue_not_full,NULL);


    for(int i = 0; i < max_thread_num; i++)
    {
       Thread * t =  InitThread(thread_woker,p);
       DBInsertTail(&p->DL_thread,t);                  
    }

    p->admin_thread = InitThread(thread_manager,p);

    return p;
}

void Thread_Add_Task(ThreadP *p, void *(*function)(void *, void *), void *arg1, void *arg2)
{
    pthread_mutex_lock(&p->pool_mutex);                      // 操作队列，公共资源，先加锁

    /* 当任务队列已经满了,阻塞等待 */
    while (GetQueueLen(&p->task_queue) == p->max_queue_size) 
    {
        pthread_cond_wait(&p->queue_not_full, &p->pool_mutex); 
    }

    if (p->shutDown == true) 
    {
        pthread_mutex_unlock(&p->pool_mutex); 
        return;
    }

    QPush(&p->task_queue, CreateTask(function, arg1,arg2)); // 任务入队列

    pthread_cond_broadcast(&p->queue_not_empty);  // 解锁之前给线程发信号来执行
    pthread_mutex_unlock(&p->pool_mutex);         // 解锁
}

void Destory_Thread_Pool(ThreadP *p)
{
   
}
