#ifndef _QUEUE_H_
#define _QUEUE_H_
#include"DoubleLinkList.h"

struct LinkQueue//队列
{
    DLlist queue;
    ElementType FrontData;//
};
typedef struct LinkQueue LQueue;

int InitQueue(LQueue *lq);
void QPush(LQueue *lq,ElementType element);
ElementType *Pop(LQueue *lq);
int IsQEmpty(LQueue *lq);
struct Node* GetFront(LQueue *lq);
void FreeQueue(LQueue *lq);
int GetQueueLen(LQueue *q);//获取队列长度
#endif

