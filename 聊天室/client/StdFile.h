#ifndef __STDFILE_H_
#define __STDFILE_H_
#include<stddef.h>
#include"DoubleLinkList.h"
int IsFileExist(const char *FilePath);

char *LoadFromFile(const char *FilePath);//读文件里面的东西,返回一个堆上的数组，调用之后要free
DLlist *GetLineFromFile(const char *FilePath);//从文件中逐行读取内容（整个文件），并将每一行保存到一个双向链表中
void WriteToFile(const char * FilePath,void * ptr,size_t size);//写入指定文件
void WriteLineToFile(const char * FilePath,DLlist *list);//一行一行写，将双向链表中的每一行内容写入到指定的文件中
void AppendToFile(const char *FilePath,void *ptr,size_t size);//追加写
void CopyFile(const char *SourcePath,const char *TargetPath);
#endif