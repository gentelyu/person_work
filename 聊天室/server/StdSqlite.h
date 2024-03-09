#ifndef __STDSQLITE_H_
#define __STDSQLITE_H_

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "DoubleLinkList.h"


typedef struct StdSqlite SQL;

SQL*InitSqlite(const char *filename);
void CreateTable(SQL*s,const char *tableName,char **prolist,int row);
void DeleteTable(SQL*s,const char *tableName);
void InsertData(SQL *s, const char *tableName, char *count, char *name, char *password);
void DeleteData(SQL *s,const char *tableName,const char *where);
void UpdateData(SQL *s,const char *tableName,const char *SetValue,const char *where);
void  GetTableInfo(SQL*s,const char *tableName,char ***result,int *row,int *column);
int SelectInfo(SQL *s,const char *sql,char ***result,int *row,int *column);
char *GetTableData(SQL *s, const char *tableName,char *account,char *passwd);
void FreeInfoReault(char **result);
int GetTableLen(SQL *s,const char *tableName);
int IsTableEmpty(SQL *s,const char *tableName);

sqlite3 *GetSqlDb(SQL *s);//获取da


void FreeSqlite(SQL *s);

#endif