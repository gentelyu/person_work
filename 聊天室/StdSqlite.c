#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include "StdSqlite.h"
#include "DoubleLinkList.h"

#define false 0
#define true 1

struct StdSqlite
{
    sqlite3 *db;
};

SQL *InitSqlite(const char *filename)
{
    SQL *s = (SQL *)malloc(sizeof(SQL));
    if (s == NULL)
    {
        printf("initSqlite malloc error\n");
        return NULL;
    }

    if (sqlite3_open(filename, &s->db) != SQLITE_OK)
    {
        printf("open %s error:%s", filename, sqlite3_errmsg(s->db));
        free(s);
        return NULL;
    }
    return s;
}

void CreateTable(SQL *s, const char *tableName, char **prolist, int row)
{
    char property[1024] = {0};
    for (int i = 0; i < row; i++)
    {
        strcat(property, prolist[i * 2]);
        strcat(property, " ");
        strcat(property, prolist[i * 2 + 1]);
        if (i != row - 1)
        {
            strcat(property, ",");
        }
    }
    char sql[4096] = {0};
    sprintf(sql, "create table %s(%s);", tableName, property);
    printf("%s\n",sql);
    if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("create table error: %s\n", sqlite3_errmsg(s->db));
    }
}

void  DeleteTable(SQL *s, const char *tableName)
{
    char sql[4096] = {0};
    sprintf(sql,"drop table %s;",tableName);
    if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("delete table error: %s\n", sqlite3_errmsg(s->db));
    }
}

// void InsertData(SQL *s, const char *tableName, char **values, int size)
// {
//     char valueList[1024];
//     for (int i = 0; i < size; i++)
//     {
//         strcat(valueList, values[i]);
//         if (i != size - 1)
//         {
//             strcat(valueList, ",");
//         }
//     }
//     char sql[4096] = {0};
//     sprintf(sql, "insert into %s values(%s);", tableName, valueList);
//     if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
//     {
//         printf("insert data error: %s\n", sqlite3_errmsg(s->db));
//     }
// }
void InsertData(SQL *s, const char *tableName, char *count, char *name, char *password)
{
    char sql[4096] = {0};
    sprintf(sql, "insert into %s values('%s','%s','%s');", tableName, count, name, password);
    if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("insert table error :%s\n!", sqlite3_errmsg(s->db));
    }
}
void InsertDatafrined(SQL *s, const char *tableName, char *count, char *name, char *password)
{
    char sql[4096] = {0};
    sprintf(sql, "insert into %s values('%s','%s','%s');", tableName, count, name, password);
    if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("insert table error :%s\n!", sqlite3_errmsg(s->db));
    }
}

void DeleteData(SQL *s, const char *tableName, const char *where)
{
    char sql[4096] = {0};
    if (where == NULL)
    {
        sprintf(sql, "delete from %s;", tableName);
    }
    else
    {
        sprintf(sql, "delete from %s where %s", tableName, where);
    }
    if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("delete data error: %s\n", sqlite3_errmsg(s->db));
    }
}

void UpdateData(SQL *s, const char *tableName, const char *SetValue, const char *where)
{
    char sql[4096] = {0};
    sprintf(sql, "update %s set %s where %s", tableName, SetValue, where);

    //printf("%s\n",sql);
    if (sqlite3_exec(s->db, sql, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("update data error: %s\n", sqlite3_errmsg(s->db));
    }
}

void GetTableInfo(SQL *s, const char *tableName, char ***result, int *row, int *column)
{
    char sql[4096] = {0};
    sprintf(sql, "select *from %s", tableName);
    if (sqlite3_get_table(s->db, sql, result, row, column, NULL) != SQLITE_OK)
    {
        printf("GetTableInfo error:%s", sqlite3_errmsg(s->db));
    }
}

int SelectInfo(SQL *s, const char *sql, char ***result, int *row, int *column)//
{
    if (sqlite3_get_table(s->db, sql, result, row, column, NULL) != SQLITE_OK)
    {
        printf("SelectInfo error:%s", sqlite3_errmsg(s->db));
        return false;
    }
    return true;
}

char *GetTableData(SQL *s, const char *tableName,char *account,char *passwd)
{
    char sql[4096] = {0};
    sprintf(sql, "select *from %s", tableName);
    char **result;
    int row, column;
    if (sqlite3_get_table(s->db, sql, &result, &row, &column, NULL) != SQLITE_OK)
    {
        printf("GetTableLen error:%s", sqlite3_errmsg(s->db));
    }
    for(int i = 0;i <=row;i ++)
    {
        for(int j = 0;j < column;j++)
        {
            printf("%s|",result[i*column+j]);
        }
        printf("\n");
    }
    for(int i = 1;i <= row;i++)
    {
        if(strcmp(result[i*column],account) == 0 && strcmp(result[i*column+1],passwd) == 0)
        {
            return result[i*column+2];
        }
    }
    return NULL;
    
   
}

void FreeInfoReault(char **result)
{
    sqlite3_free_table(result);
}

int GetTableLen(SQL *s, const char *tableName)
{
    char sql[4096] = {0};
    sprintf(sql, "select count(*) from %s", tableName);

    char **result;
    int row, column;
    if (sqlite3_get_table(s->db, sql, &result, &row, &column, NULL) != SQLITE_OK)
    {
        printf("GetTableLen error:%s", sqlite3_errmsg(s->db));
    }
    int len = atoi(result[column]);
    sqlite3_free_table(result);
    return len;
}

int IsTableEmpty(SQL *s, const char *tableName)
{
    if(GetTableLen(s,tableName) > 0)
    {
        return false;
    }
    return true;
}

sqlite3 *GetSqlDb(SQL *s)
{
    return s->db;
}

void FreeSqlite(SQL *s)
{
    if (s == NULL)
    {
        return;
    }
    sqlite3_close(s->db);
    free(s);
}
