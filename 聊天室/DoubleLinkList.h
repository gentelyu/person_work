#ifndef _DOUBLELINKLIST__H_
#define _DOUBLELINKLIST__H_
#define ElementType void*

struct Node
{
    ElementType data;
    struct Node *next;
    struct Node *prev;
};

struct DoubleLinkList //没有头结点的双向链表
{
    struct Node *head;
    struct Node *tail;
    int len;
};

typedef struct DoubleLinkList DLlist;

int InitDLlist(DLlist *list);//初始化
void DBInsertTail(DLlist *list,ElementType element);//尾插
void DBInsertHead(DLlist *list,ElementType element);//头插
void DBInsertByIndex(DLlist *list,int index, ElementType element);//按位插
void DBRemoveByIndex(DLlist *list,int index);//按位删
void DBRemoveByElement(DLlist *list,ElementType element);//按值删
int DBFindeFirstByElement(DLlist *list,ElementType element);//找某个元素第一次出现的位置
ElementType *DBFindeByindex(DLlist *list,int index);//按位查找
int *DBFindByElement(DLlist *list,ElementType element);//按值查找
void DBSeValueByIndex(DLlist *list,int index,ElementType element);//按下标修改
void DBSeValueByElent(DLlist *list,ElementType oidValue,ElementType newValue);
void DBTravel(DLlist *list);//打印链表的值
void FreeDLlist(DLlist *list);//删除整条链表的操作
int GetDBlistLen(DLlist *list);



#endif
