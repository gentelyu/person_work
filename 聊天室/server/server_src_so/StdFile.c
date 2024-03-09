#include"StdFile.h"//
#include<stdio.h>
#include<stdlib.h>
#include"DoubleLinkList.h"
#include<string.h>
#include<stddef.h>
#define true 1
#define false 0
int IsFileExist(const char *FilePath)//检查文件是否存在
{//看参数
    FILE *file = fopen(FilePath,"r");
    if(file == NULL)
    {
        return false;
    }
    else
    {
        fclose(file);
        return true;
    }
}

char *LoadFromFile(const char *FilePath)
{
    FILE * file = fopen(FilePath,"r");
    if(file == NULL)
    {
        printf("file open fail!\n");
        return NULL;
    }
    else
    {
        printf("file open success!\n");
    }
    fseek(file,0,SEEK_END);//将光标放到文件末尾
    size_t size =  ftell(file);//获取文件的大小，返回值为long型的数字，刚好为文件里面字符的个数
    char * content = (char *)malloc(size+1);
    //为内容分配内存，大小为文件大小加上一个额外的字节用于字符串的结束标记 '\0'。
    fseek(file,0,SEEK_SET);//将文件位置指针移动回文件开头
    fread(content,size,1,file);//从文件中读取内容，将其存储在刚分配的内存块中
    content[size] = '\0';
    fclose(file);
    return content;
}

void ClearChar(char *str,char c)//去掉  的接口
{

}

void RemoveCharIndex(char *a, int index)//
{
    if(index < 0 || index >= strlen(a))
    {
        printf("Removeindex invalid place!\n");
        return;
    }
    for(int i= index;i < strlen(a);i++)
    {
        a[i] = a[i+1];
    }
}    

void RemoveChar(char *a,char element)//
{
    int len = strlen(a);
    for(int i = 0;i < len;i++)
    {
        if(a[i] == element)
        {
            RemoveCharIndex(a,i);
            i--;
        }
    }
}

DLlist *GetLineFromFile(const char *FilePath)//从文件中逐行读取内容（整个文件），并将每一行保存到一个双向链表中
{
    FILE * file = fopen(FilePath,"r");
    if(file == NULL)
    {
        printf("file open fail!\n");
        return NULL;
    }

    DLlist * list = (DLlist *)malloc(sizeof(DLlist));//为链表申请空间
    InitDLlist(list);//初始化链表

    char ContentTemp[100] = {0};//用一块数组来接文件中的字符串
    while(fgets(ContentTemp,100,file) != NULL)//将文件里的字符串读取到数组中
    {
        char * Line = (char *)malloc(strlen(ContentTemp)+1);//申请一块和数组中有效数据相同大小的空间，+1是给‘\0’留个位置
        strcpy(Line,ContentTemp);//将数组中的数据拷贝到Line中
        RemoveChar(Line,'\n');//将‘\n’去掉为了好读取账号密码
        DBInsertTail(list,Line);//将数据插入到链表中
    }
    fclose(file);
    return list;
}

void WriteToFile(const char *FilePath, void *ptr, size_t size)//没有返回值的写入
{
    FILE *file = fopen(FilePath,"w");//清空写
    if(file == NULL)
    {
        printf("writeToFile open file error!\n");
        return;
    }
    if (fwrite(ptr,size,1,file) <= 0)
    {
        printf("WriteToFile error!\n");
    }
    fclose(file);
}

void WriteLineToFile(const char *FilePath, DLlist *list)
{
    //果果文件不存在就创建，存在就清空里面的内容
    FILE *file = fopen(FilePath,"w");
    if(file == NULL)
    {

        printf("writeToFile open file error!\n");
        return;
    }

    struct Node * TravelPoint = list->head;
    while(TravelPoint != NULL)
    {
        fputs(TravelPoint->data,file);//要改成自己的接口，实现回调
        TravelPoint = TravelPoint->next;
    }
    fclose(file);
}

void AppendToFile(const char *FilePath, void *ptr, size_t size)//追加写
{
    //FilePath：文件路径，指定要追加写入的文件位置和名称。
    //ptr：指向数据的指针，需要写入到文件中的数据。
    //size：数据的大小，以字节为单位。
    FILE *file = fopen(FilePath,"a");
    if(file == NULL)
    {
        printf("AppendToFile open file error!\n");
        return;
    }

    if(fwrite(ptr,size,1,file) <= 0)
    {
        printf("AppendToFile error!\n");
    }
    fclose(file);
}

void CopyFile(const char *SourcePath, const char *TargetPath)//将目标文件复制到目标位置，用于复制
{
    if(IsFileExist(SourcePath) == false)
    {//使用 IsFileExist 函数检查源文件是否存在。如果源文件不存在或没有读取权限，输出错误信息并返回。
        printf("the sourcefile is not exist or has no read permission!\n");
        return;
    }
    //调用 LoadFromFile 函数加载源文件的内容，并将内容存储在名为 fileContent 的字符指针中。
    char *fileContent = LoadFromFile(SourcePath);

    if(IsFileExist(TargetPath) == true)//判断是否冲突
    {
        char Target[100] = {0};
        strcpy(Target,TargetPath);
        char *fileName = strtok(Target,".");//切割函数
        char *backName = strtok(NULL,".");

        char NewPath[100] = {0};
        strcpy(NewPath,fileName);
        strcat(NewPath,"_new.");
        strcat(NewPath,backName);
        if(IsFileExist(NewPath) == true)//如果改过之后的名字还是重复，递归
        {
            CopyFile(SourcePath,NewPath);
            return;
        }
        WriteToFile(NewPath,fileContent,strlen(fileContent));
        free(fileContent);
        return;
    }

    WriteToFile(TargetPath,fileContent,strlen(fileContent));//有个问题，文件名冲突
    free(fileContent);//释放
}
