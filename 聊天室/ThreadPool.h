#ifndef __THREADPOOL_H_
#define __THREADPOLL_H_

#include "StdSqlite.h"
typedef struct StdSqlite SQL;

typedef struct ThreadPool ThreadP;

ThreadP * InitThreadPool(int max_thrd_num,int min_thrd_num,int max_queue_size);

void Threadp_AddTask(ThreadP * p,void *(*func)(void *,void *),void *arg1,void *arg2);//塞任务,第二个参数是回调函数，放任务函数，

void DestoryThreadPool(ThreadP * p);






#endif