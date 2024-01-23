#include"DoubleLinkList.h"
#include<stdio.h>
#include<stdlib.h>

int InitDLlist(DLlist *list)
{
    list->head = NULL;
    list->len = 0;
    list->tail = NULL;
    return 0;
}

struct Node *CreateNode(ElementType element)
{
    struct Node *newnode = (struct Node *)malloc(sizeof(struct Node));
    if(newnode == NULL)
    {
        printf("Create newnode error!\n");
        return NULL;
    }

    newnode->data = element;
    newnode->next = NULL;
    newnode->prev = NULL;
    return newnode;
}

void DBInsertTail(DLlist *list, ElementType element)
{
    struct Node *newNode = CreateNode(element);
    if(newNode == NULL)
    {
        printf("InsertTail Create error!\n");
        return;
    }

    if(list->len == 0)
    {
        list->head = newNode;
        list->tail = newNode;
    }
    else
    {
        list->tail->next = newNode;
        newNode->prev = list->tail;
        list->tail = newNode;
    }
    list->len++;
}

void DBInsertHead(DLlist *list, ElementType element)
{
    struct Node *newNode = CreateNode(element);
    if(newNode == NULL)
    {
        printf("InsertHead Create error!\n");
        return ;
    }

    if(list->len == 0)
    {
        list->head = newNode;
        list->tail = newNode;
    }
    else
    {
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    }
    list->len++;
}

void DBInsertByIndex(DLlist *list, int index, ElementType element)//按位置插入
{
    if(index<0 || index> list->len)
    {
        printf("InsertByIndex invalid place!\n");
        return;
    }
    if(index == 0)
    {
        DBInsertHead(list,element);  
        return;
    }
    if(index == list->len)
    {
        DBInsertTail(list,element);
        return;
    }
    if(index > 0 && index < list->len)
    {
        struct Node *newNode = CreateNode(element);
        struct Node *TravelPoint = list->head;
        while(index != 0)
        {
            TravelPoint = TravelPoint->next;
            index--;
        }
        newNode->prev = TravelPoint->prev;
        newNode->next = TravelPoint;
        TravelPoint->prev->next = newNode;
        TravelPoint->prev = newNode;
        list->len++;
    }
}

void DBRemoveByIndex(DLlist *list, int index)
{
    if(index < 0 || index > list->len-1)
    {
        printf("Remve Index invalid place!\n");
        return;
    }
    if(index == 0)
    {
        if(list->len == 1)
        {
            free(list->head);
            list->head = NULL;
            list->tail = NULL;
            list->len = 0;
            return;
        }
        struct Node *freenode = list->head;
        list->head = list->head->next;
        list->head->prev = NULL;
        free(freenode);
        list->len--;
        return;
    }

    if(index == list->len-1)
    {
        struct Node *freeNode = list->tail;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
        free(freeNode);
        list->len--;
        return;
    }
    struct Node *TravelPoint = list->head;
    while(index > 0)
    {
        TravelPoint = TravelPoint->next;
        index--;
    }
    TravelPoint->prev->next = TravelPoint->next;
    TravelPoint->next->prev = TravelPoint->prev;
    free(TravelPoint);
    list->len--;
}

void DBRemoveByElement(DLlist *list, ElementType element)
{
    int index = DBFindeFirstByElement(list,element);
    while(index != -1)
    {
        DBRemoveByIndex(list,index);
        index = DBFindeFirstByElement(list,element);//  这一步是干嘛的？
    }

}

int DBFindeFirstByElement(DLlist *list, ElementType element)
{
    int count = 0;
    struct Node *TravelPoint = list->head;
    while(TravelPoint != NULL)
    {
        if(TravelPoint->data == element)
        {
            return count;
        }
        count++;
        TravelPoint = TravelPoint->next;
    }
    return -1;//找不到时返回非法值
}

ElementType *DBFindeByindex(DLlist *list, int index)
{
    if(index < 0 || index > list->len-1)
    {
        printf("FindByindex invalid place!\n");
        printf("请重新查找!\n");
        return NULL;
    }
    struct Node *TravelPoint = list->head;
    while(index != 0)
    {
        TravelPoint = TravelPoint->next;
        index--;
    }
    return &TravelPoint->data;
}

int *DBFindByElement(DLlist *list, ElementType element)
{
    int *FindVector = (int *)malloc(sizeof(int) * (list->len+1));
    if(FindVector == NULL)
    {
        printf("FindVector malloc error!\n");
        return NULL;
    }
    int count = 0;
    int i = 0;
    struct Node *TravelPoint = list->head;
    while(TravelPoint != NULL)
    {
        if(TravelPoint->data == element)
        {
            count++;
            FindVector[count] = i;
        }
        i++;
        FindVector[0] = count;
        TravelPoint = TravelPoint->next;
    }
    FindVector[i] = -1;//加不加
    return FindVector;
}

void DBSeValueByIndex(DLlist *list, int index, ElementType element)
{
    if(index < 0 || index > list->len-1)
    {
        printf("SaValueByIndex Invalid place!\n");
        return;
    }
    struct Node *TravelPoint = list->head;
    while(index != 0)
    {
        TravelPoint = TravelPoint->next;
        index--;
    }
    TravelPoint->data = element;
}

void DBSeValueByElent(DLlist *list, ElementType oidValue, ElementType newValue)
{
    int index = 0;
    while(index != -1)
    {
        index = DBFindeFirstByElement(list,oidValue);
        DBSeValueByIndex(list,index,newValue);
        index = DBFindeFirstByElement(list,oidValue);
    }
}

void DBTravel(DLlist *list)
{
    printf("len : %d\n",list->len);
    printf("next : ");
    struct Node *TravelPoint = list->head;
    while(TravelPoint != NULL)
    {
        //printf("%d ",TravelPoint->data);
        TravelPoint = TravelPoint->next;
    }
    printf("\n");

    printf("Prev : ");
    TravelPoint = list->tail;
    while(TravelPoint != NULL)
    {
        //printf("%d ",TravelPoint->data);
        TravelPoint = TravelPoint->prev;
    }
    printf("\n");
}

void FreeDLlist(DLlist *list)
{
    while(list->head != NULL)
    {
        struct Node *freeNode = list->head;
        list->head = list->head->next;
        free(freeNode);
    }
    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
}

int GetDBlistLen(DLlist *list)
{
    return list->len;
}
