// g++ datatest.cpp -o testpq -lpq

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <random>
#include <set>
#include <ctime>
#include <fstream>
#include <sstream>
#include <time.h>

#include "../include/DBOGaccess.hpp"

#define FILEID_SIZE 16 // 每个文件id的长度
#define DB_SIZE 120 // 数据库大小

using namespace std;

int main() {

    const char* paramValues[1];

    vector<uint8_t> data = {0x00, 0xa1, 0xd9};
    // unsigned char data[3] = {0x00, 0x0d, 0xd9}; // 示例数据
    // char* Value = new char[8];
    // std::memcpy(Value, data.data(), data.size());
    // paramValues[0] = Value; // 将字节向量转换为字符指针
    paramValues[0] = (const char*)data.data(); // 将字节向量转换为字符指针
    // paramValues[0] = reinterpret_cast<const char*>(data.data());
    
    DBOGaccess db1;
    
    // 设置参数长度
    int paramLengths[1];
    paramLengths[0] = sizeof(data);

    cout<< "paramLengths[0]: " << paramLengths[0] << endl;

    // 设置二进制格式
    int paramFormats[1];
    paramFormats[0] = 1; // 1 表示二进制格式

    // SQL 插入命令
    const char* command = "INSERT INTO t2 (vol1) VALUES ($1);";

    // 执行插入操作
    PGresult* res = PQexecParams(
        db1.conn,             // 连接对象
        command,          // SQL 命令
        1,                // 参数的数量
        NULL,          // 参数类型（NULL 表示自动推断）
        paramValues,      // 参数值
        paramLengths,     // 参数长度
        paramFormats,     // 参数格式（1表示二进制格式）
        1                 // 是否返回结果（0表示不返回）
    );

    // 检查是否插入成功
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Insert failed: " << PQerrorMessage(db1.conn) << std::endl;
    } else {
        std::cout << "Data inserted successfully!" << std::endl;
    }

    // 清理
    PQclear(res);

    return 0;
}
