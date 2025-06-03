// g++ testpq.cpp -o testpq -lpq

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <time.h>

#define BYTE_SIZE 16 // 每个字节向量的长度

using namespace std;

// 生成随机字节向量数据
void generate_random_bytes(unsigned char *buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = rand() % 256;
    }
}

int main() {
    // 连接数据库
    PGconn *conn = PQconnectdb("host=127.0.0.1 port=15432 user=cpp_md5 dbname=postgres password=cpp123");

    // 检查连接是否成功
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("连接失败: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }
    printf("数据库连接成功\n");

    // 生成3个随机字节向量
    unsigned char data1[BYTE_SIZE], data2[BYTE_SIZE], data3[BYTE_SIZE];
    generate_random_bytes(data1, BYTE_SIZE);
    generate_random_bytes(data2, BYTE_SIZE);
    generate_random_bytes(data3, BYTE_SIZE);
    printf("随机字节向量生成完毕\n");
    cout<<"sizeof data1="<<sizeof(data1)<<endl;

    // 创建 SQL 插入命令（确保列名大小写匹配）
    const char *insert_query = "INSERT INTO cipher (\"location\", \"mw\", \"mid\") "
                           "VALUES ($1, $2, $3) ";

    // 插入数据
    const int nParams = 3; // 三个字节向量
    // Oid paramTypes[] = NULL; 
    const char *paramValues[] = {
        (const char *)data1, 
        (const char *)data2, 
        (const char *)data3
    };
    int paramLengths[] = {BYTE_SIZE, BYTE_SIZE, BYTE_SIZE};  // 每个字节向量的长度
    int paramFormats[] = {1, 1, 1};  // 1=二进制格式，0=文本格式

    printf("正在插入数据...\n");
    PGresult *res = PQexecParams(conn, insert_query, nParams, NULL, paramValues, paramLengths, paramFormats, 1);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("插入数据失败: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return 1;
    }
    PQclear(res);
    printf("数据插入成功\n");

    // 查询刚插入的数据（也使用 PQexecParams）
    const char *select_query = "SELECT \"location\", \"mw\", \"mid\" FROM cipher LIMIT 1";
    printf("正在查询数据...\n");
    res = PQexecParams(conn, select_query, 0, NULL, NULL, NULL, NULL, 1); // 最后一个参数为1表示返回二进制类型

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("查询数据失败: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // 获取查询的结果
    unsigned char read_data1[BYTE_SIZE], read_data2[BYTE_SIZE], read_data3[BYTE_SIZE];

    memcpy(read_data1, PQgetvalue(res, 0, 0), BYTE_SIZE);
    memcpy(read_data2, PQgetvalue(res, 0, 1), BYTE_SIZE);
    memcpy(read_data3, PQgetvalue(res, 0, 2), BYTE_SIZE);

    // 打开文件用于写入
    FILE *file = fopen("cipher_data.txt", "w");
    if (file == NULL) {
        printf("无法打开文件\n");
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // 写入插入的字节数据到文件
    fprintf(file, "插入的字节数据:\n");
    for (int i = 0; i < BYTE_SIZE; i++) {
        fprintf(file, "%02X ", data1[i]);
    }
    fprintf(file, "\n");
    for (int i = 0; i < BYTE_SIZE; i++) {
        fprintf(file, "%02X ", data2[i]);
    }
    fprintf(file, "\n");
    for (int i = 0; i < BYTE_SIZE; i++) {
        fprintf(file, "%02X ", data3[i]);
    }
    fprintf(file, "\n");

    // 写入读取的字节数据到文件
    fprintf(file, "读取的字节数据:\n");
    for (int i = 0; i < BYTE_SIZE; i++) {
        fprintf(file, "%02X ", read_data1[i]);
    }
    fprintf(file, "\n");
    for (int i = 0; i < BYTE_SIZE; i++) {
        fprintf(file, "%02X ", read_data2[i]);
    }
    fprintf(file, "\n");
    for (int i = 0; i < BYTE_SIZE; i++) {
        fprintf(file, "%02X ", read_data3[i]);
    }
    fprintf(file, "\n");

    // 清理和关闭
    fclose(file);
    PQclear(res);
    PQfinish(conn);

    printf("数据已写入 cipher_data.txt\n");
    return 0;
}
