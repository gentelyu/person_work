#include "Queue.h"
#include<stdio.h>
#define true 1
#define false 0

int InitQueue(LQueue *lq)//初始化
{
    return InitDLlist(&lq->queue);      
}

void QPush(LQueue *lq, ElementType element)//入队列
{
    DBInsertTail(&lq->queue,element);
}

ElementType *Pop(LQueue *lq)//出队列
{
    if(lq->queue.len == 0)
    {
        printf("Queue is Empty!\n");
        return NULL;
    }

    lq->FrontData = lq->queue.head->data;
    DBRemoveByIndex(&lq->queue,0);
    return &lq->FrontData;
}

int IsQEmpty(LQueue *lq)
{
    if(lq->queue.len == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
    return 0;
}

struct Node *GetFront(LQueue *lq)
{
    return lq->queue.head;
    
}

void FreeQueue(LQueue *lq)
{
    FreeDLlist(&lq->queue);
}

int GetQueueLen(LQueue *q)
{
    return q->queue.len;

}
