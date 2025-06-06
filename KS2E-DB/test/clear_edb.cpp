// g++ clear_edb.cpp -o testpq -lpq

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
    DBOGaccess db1;
    PGresult* res = nullptr;

    vector<vector<uint8_t>> datavecs;

    // 插入数据
    const char* command_insert = "INSERT INTO encrypted_index_table (location) VALUES (E'\\x000dd926973a354b');";
    db1.writeData(datavecs, command_insert, 1);


    // 清理
    if (res != nullptr) {
        PQclear(res);
    }

    return 0;
}
