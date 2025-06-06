// g++ DBOGdemo.cpp -o testpq -lpq

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include "DBOGaccess.hpp"

#define BYTE_SIZE 16 // 每个字节向量的长度

using namespace std;

void printVectors(const std::vector<std::vector<uint8_t>>& datavecs) {
    for (size_t i = 0; i < datavecs.size(); ++i) {
        for (size_t j = 0; j < datavecs[i].size(); ++j) {
            // 打印每个字节，使用十六进制格式
            std::cout << "\\x" << std::setw(2) << std::setfill('0') 
                      << std::hex << (int)datavecs[i][j];
        }
        std::cout << " ";
    }
    cout<<endl;
}

int main() {
    DBOGaccess db1;
    PGresult* res = nullptr;

    // 查询数据
    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    vector<vector<uint8_t>> datavecs = {{0x01,0x02}, {0x02,0x03}, {0x03,0x04,0x05}}; // 示例数据
    vector<vector<uint8_t>> datavecs2 = {{0x04}, {0x05}, {0x06}};
    vector<vector<uint8_t>> query_data = {{0x01,0x02}};
    vector<vector<uint8_t>> query_keyword = {{'2'}};

    // const char* command_create_table = "CREATE TABLE IF NOT EXISTS t2 (vol1 BYTEA PRIMARY KEY, vol2 BYTEA, vol3 BYTEA);";
    // const char* command_insert = "INSERT INTO t2 (vol1, vol2, vol3) VALUES ($1, $2, $3);";
    const char* command_select1 = "SELECT vol1,vol2,vol3 FROM t2 WHERE vol1 = $1;";
    const char* command_select2 = "SELECT * FROM cigar where num = $1;";
    // const char* command_delete = "DELETE FROM t2 WHERE vol1 = $1;";
    // const char* command_clear_table = "TRUNCATE TABLE t2;";
    // const char* command_drop_table = "DROP TABLE IF EXISTS t2;";
    // const char* command_showversion = "SELECT version();";

    // db1.writeData(datavecs, command_insert, 1);

    vector<vector<vector<uint8_t>>> r1 = db1.readData(res, query_data, command_select1, 1, 1);
    for(size_t i=0;i<r1.size();i++) {
        printVectors(r1[i]);
    }

    vector<vector<vector<uint8_t>>> r2 = db1.readData(res, query_keyword, command_select2, 1, 0);
    for (const auto& entry : r2) {
        for (const auto& element : entry) {
            // 将字节向量转换为字符串
            string str(element.begin(), element.end());
            cout << str << " ";
        }
        cout << endl;
    }

    // db1.createTable(command_create_table);
    // db1.writeData(datavecs, command_insert);
    // db1.alterData(datavecs2, command_insert);
    // db1.deleteData(datavecs, command_delete);
    // db1.clearTable(command_clear_table);
    // db1.dropTable(command_drop_table);

    // 清理
    if (res != nullptr) {
        PQclear(res);
    }

    return 0;
}
