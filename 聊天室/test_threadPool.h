#ifndef TEST_THREAD_POOL_H
#define TEST_THREAD_POOL_H

typedef struct threadPool ThreadP;

ThreadP * InitThreadPool(int max_thread_num,int min_thread_num,int max_queue_size);

void Thread_Add_Task(ThreadP *p,void *(*function)(void *,void *),void *arg1,void *arg2);

void Destory_Thread_Pool(ThreadP * p);

#endif