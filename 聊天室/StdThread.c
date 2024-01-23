#include"StdThread.h"
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

struct StdThread
{
    pthread_t threadID;
};

Thread *InitThread(void *(*func)(void *), void *arg)
{
    Thread *t = (Thread *)malloc(sizeof(Thread));
    if(t == NULL)
    {
        printf("InitThread malloc error!\n");
        return NULL;
    }
    if(pthread_create(&t->threadID,NULL,func,arg) != 0)
    {
        perror("pthread_create():");
        free(t);
        return NULL;
    }
    return t;
}

unsigned long int GetThreadID(Thread *t)
{
    return t->threadID;
}

void *JoinThread(Thread *t)
{
    void *value = NULL;
    pthread_join(t->threadID,&value);
    return value;
}

void DetachThread(Thread *t)
{
    if(pthread_detach(t->threadID) != 0)
    {
        perror("pthread_detach():");
    }
}

void CancelThread(Thread *t)
{
    if(pthread_cancel(t->threadID) != 0)
    {
        perror("pthread_cancel():");
    }
}


void ClearThread(Thread *t)
{
    free(t);
}
