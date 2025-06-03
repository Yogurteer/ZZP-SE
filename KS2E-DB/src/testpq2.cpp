// g++ testpq2.cpp -o testpq -lpq

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <random>
#include <time.h>
#include <vector>
#include <string>

#define BYTE_SIZE 8 // 每个字节向量的长度

using namespace std;

// 生成随机字节向量数据
void generate_random_bytes(unsigned char *buffer, int size) {
    // 初始化随机数生成器
    std::random_device rd;  // 获取硬件随机数种子
    std::mt19937 gen(rd());  // 使用 Mersenne Twister 引擎
    std::uniform_int_distribution<> dis(0, 255);  // 定义随机数范围 [0, 255]

    // 生成随机字节并填充到缓冲区
    for (int i = 0; i < size; i++) {
        buffer[i] = dis(gen);  // 生成一个在 [0, 255] 之间的随机数
    }
}

PGresult* Myinsert(PGconn *conn, const char *insert_query, int nParams, const std::vector<std::vector<unsigned char>> &paramValues) {
    // 创建动态数组，用于存储转换后的 const char* 类型的参数
    const char *paramPointers[nParams];
    int paramLengths[nParams];  // 存储每个参数的长度
    int paramFormats[nParams];  // 存储每个参数的格式（全设为二进制格式）

    // 填充 paramPointers 和 paramLengths 数组
    for (int i = 0; i < nParams; ++i) {
        paramFormats[i] = 1;  // 设置所有参数为二进制格式

        // 获取当前 vector 的大小并计算长度
        paramLengths[i] = paramValues[i].size();

        // 转换为 const char*，确保参数是合适的类型
        paramPointers[i] = reinterpret_cast<const char*>(paramValues[i].data());
    }

    // 执行 SQL 插入操作
    PGresult *res = PQexecParams(conn, insert_query, nParams, NULL, paramPointers, paramLengths, paramFormats, 1);

    return res;  // 返回执行结果
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

    printf("正在插入数据...\n");

    for(int i = 0; i < 20; i++) {
        // 生成3个随机字节向量
        unsigned char data1[BYTE_SIZE], data2[BYTE_SIZE*2], data3[BYTE_SIZE*2];
        generate_random_bytes(data1, BYTE_SIZE);
        generate_random_bytes(data2, BYTE_SIZE*2);
        generate_random_bytes(data3, BYTE_SIZE*2);

        // 创建 SQL 插入命令（确保列名大小写匹配）
        const char *insert_query = "INSERT INTO cipher (\"location\", \"mw\", \"mid\") "
                               "VALUES ($1, $2, $3) ";

        vector<unsigned char> v1(data1, data1 + BYTE_SIZE);
        vector<unsigned char> v2(data2, data2 + BYTE_SIZE*2);
        vector<unsigned char> v3(data3, data3 + BYTE_SIZE*2);

        vector<vector<unsigned char>> paramValues = {v1, v2, v3};

        // 插入数据
        PGresult *res = Myinsert(conn, insert_query, 3, paramValues);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("插入数据失败: %s", PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            return 1;
        }
        PQclear(res);
    }

    

    // 查询刚插入的数据（也使用 PQexecParams）
    const char *select_query = "SELECT \"location\", \"mw\", \"mid\" FROM cipher LIMIT 1";
    printf("正在查询数据...\n");
    PGresult *res1 = PQexecParams(conn, select_query, 0, NULL, NULL, NULL, NULL, 1); // 最后一个参数为1表示返回二进制类型

    if (PQresultStatus(res1) != PGRES_TUPLES_OK) {
        printf("查询数据失败: %s", PQerrorMessage(conn));
        PQclear(res1);
        PQfinish(conn);
        return 1;
    }

    // 获取查询的结果
    unsigned char read_data1[BYTE_SIZE], read_data2[BYTE_SIZE], read_data3[BYTE_SIZE];

    memcpy(read_data1, PQgetvalue(res1, 0, 0), BYTE_SIZE);
    memcpy(read_data2, PQgetvalue(res1, 0, 1), BYTE_SIZE);
    memcpy(read_data3, PQgetvalue(res1, 0, 2), BYTE_SIZE);

    // // 打开文件用于写入
    // FILE *file = fopen("cipher_data.txt", "w");
    // if (file == NULL) {
    //     printf("无法打开文件\n");
    //     PQclear(res);
    //     PQfinish(conn);
    //     return 1;
    // }

    // // 写入插入的字节数据到文件
    // fprintf(file, "插入的字节数据:\n");
    // for (int i = 0; i < BYTE_SIZE; i++) {
    //     fprintf(file, "%02X ", v1[i]);
    // }
    // fprintf(file, "\n");
    // for (int i = 0; i < BYTE_SIZE; i++) {
    //     fprintf(file, "%02X ", v2[i]);
    // }
    // fprintf(file, "\n");
    // for (int i = 0; i < BYTE_SIZE; i++) {
    //     fprintf(file, "%02X ", v3[i]);
    // }
    // fprintf(file, "\n");

    // // 写入读取的字节数据到文件
    // fprintf(file, "读取的字节数据:\n");
    // for (int i = 0; i < BYTE_SIZE; i++) {
    //     fprintf(file, "%02X ", read_data1[i]);
    // }
    // fprintf(file, "\n");
    // for (int i = 0; i < BYTE_SIZE; i++) {
    //     fprintf(file, "%02X ", read_data2[i]);
    // }
    // fprintf(file, "\n");
    // for (int i = 0; i < BYTE_SIZE; i++) {
    //     fprintf(file, "%02X ", read_data3[i]);
    // }
    // fprintf(file, "\n");

    // 清理和关闭
    // fclose(file);
    // PQclear(res);
    PQfinish(conn);

    printf("数据已写入 cipher_data.txt\n");
    return 0;
}
