// g++ gen_id.cpp -o gen_id -lpq

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

#define FILEID_SIZE 8 // 每个文件id的长度
#define DB_SIZE 120 // 数据库大小

using namespace std;

// 将字节向量转换为16进制字符串
string bytesToHex(const vector<uint8_t>& bytes) {
    stringstream ss;
    for (uint8_t byte : bytes) {
        ss << setw(2) << setfill('0') << hex << static_cast<int>(byte);
    }
    return ss.str();
}

// 生成随机的字节向量
vector<uint8_t> generateRandomFileID(size_t length) {
    vector<uint8_t> fileID(length);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 255);

    // 填充随机数据
    for (size_t i = 0; i < length; ++i) {
        fileID[i] = dist(gen);
    }

    return fileID;
}

// 函数将三维 vector 写入文件
void writeplainToFile(const vector<vector<vector<uint8_t>>>& r1, const string& filename) {
    // 打开文件，准备写入
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "无法打开文件!" << endl;
        return;
    }

    // 遍历第一维 (每一行数据)
    for (const auto& row : r1) {
        // 将对应的 id（字节向量）转换为16进制字符串并写入
        string idHex = bytesToHex(row[16]);  // 将 id 转换为 16 进制字符串

        // 遍历该行的前16个元素（关键字）
        outFile << idHex << endl;  // 写入 id 的 16 进制表示
        for (size_t i = 0; i < 16; ++i) {
            // 将关键字（字节向量）转换为字符串
            string keyword(row[i].begin(), row[i].end());
            outFile << keyword << " ";  // 写入关键字
        }
        outFile << endl;  // 换行
    }

    // 关闭文件
    outFile.close();
}

int main() {
    DBOGaccess db1;
    PGresult* res = nullptr;
    // DB dir
    char update_dir1[100] = "../result/plain.txt";

    // 更新id
    for(int i=1;i<=DB_SIZE;i++) {
        vector<uint8_t> fileID = generateRandomFileID(FILEID_SIZE);

        string numStr = to_string(i);
        // 创建一个字节向量来存储字符串的每个字符
        vector<uint8_t> num;
        for (char c : numStr) {
            num.push_back(static_cast<uint8_t>(c));  // 将字符转换为字节并存入字节向量
        }
        // num = {'\1'};

        vector<vector<uint8_t>> datavecs = {fileID, num};

        // 插入数据
        const char* command_insert = "UPDATE cigar SET id = $1 WHERE num = $2;";
        db1.writeData(datavecs, command_insert);
    }

    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    vector<vector<vector<uint8_t>>> r1 = db1.readData(res, empty_params, "SELECT * FROM cigar;", 0, 0);

    writeplainToFile(r1, update_dir1);

    // 清理
    if (res != nullptr) {
        PQclear(res);
    }

    return 0;
}
