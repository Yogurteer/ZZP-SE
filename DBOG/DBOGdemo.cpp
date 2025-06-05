// g++ DBOGdemo.cpp -o testpq -lpq

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <time.h>

#include "DBOGaccess.hpp"

#define BYTE_SIZE 16 // 每个字节向量的长度

using namespace std;

void printVector(const std::vector<std::vector<uint8_t>>& datavecs) {
    for (size_t i = 0; i < datavecs.size(); ++i) {
        std::cout << "Row " << i + 1 << ": ";
        for (size_t j = 0; j < datavecs[i].size(); ++j) {
            // 打印每个字节，使用十六进制格式
            std::cout << "0x" << std::setw(2) << std::setfill('0') 
                      << std::hex << (int)datavecs[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    DBOGaccess db1;
    PGresult* res = nullptr;

    // 查询数据
    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    vector<vector<uint8_t>> datavecs = {{0x01}, {0x02}, {0x03}}; // 示例数据
    vector<vector<uint8_t>> datavecs2 = {{0x04}, {0x05}, {0x06}};
    vector<vector<uint8_t>> query_data = {{0x01}};

    // const char* command_create_table = "CREATE TABLE IF NOT EXISTS t2 (vol1 BYTEA PRIMARY KEY, vol2 BYTEA, vol3 BYTEA);";
    // const char* command_insert = "INSERT INTO t2 (vol1, vol2, vol3) VALUES ($1, $2, $3) ON CONFLICT (vol1) DO NOTHING;";
    const char* command_select = "SELECT vol1,vol2,vol3 FROM t2 WHERE vol1 = $1;";
    // const char* command_clear_table = "TRUNCATE TABLE t2;";
    // const char* command_drop_table = "DROP TABLE IF EXISTS t2;";
    // const char* command_showversion = "SELECT version();";

    vector<vector<uint8_t>> query_result = db1.readData(query_data, command_select);
    printVector(query_result);

    // db1.createTable(command_create_table);
    // db1.writeData(datavecs, command_insert);
    // db1.dropTable(command_drop_table);

    // 清理
    if (res != nullptr) {
        PQclear(res);
    }

    return 0;
}
