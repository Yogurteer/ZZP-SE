#include <iostream>
#include <fstream>
#include <stdio.h>
#include <gmp.h>            // 用于大数运算
#include "../utils/utils.h"
#include "LPSI.h"

// sender step1:求h(y)^ba
void sender_step1(mpz_t y_ba, const mpz_t p, mpz_t y_b, const mpz_t a) {
    // 从 y_b.txt 读取十六进制数赋值给 y_b
    std::ifstream inFile("dataset/OPRF/y_b.txt");
    if (!inFile) {
        std::cerr << "Unable to open file y_b.txt" << std::endl;
        return;
    }

    // 读取文件中的十六进制数
    std::string hexString;
    std::getline(inFile, hexString); // 读取第一行
    inFile.close(); // 关闭文件

    // 从读取的十六进制字符串转换为 mpz_t
    if (mpz_set_str(y_b, hexString.c_str(), 16) != 0) {
        std::cerr << "Failed to convert hex string to mpz_t" << std::endl;
        mpz_clear(y_b); // 清理 mpz_t
        return;
    }

    // 计算 (y_b^a) mod p
    mpz_powm(y_ba, y_b, a, p);

    // 将 y_ba 的十六进制格式写入文件 y_b.txt
    FILE *outFile = fopen("dataset/OPRF/y_ba.txt", "w"); // 打开文件
    if (outFile != nullptr) {
        // fprintf(outFile, "y_b in hex: ");
        mpz_out_str(outFile, 16, y_ba);  // 以十六进制写入文件
        fprintf(outFile, "\n");
        fclose(outFile);  // 关闭文件
        // std::cout << "y_ba has been written to y_ba.txt" << std::endl;
    } else {
        std::cerr << "Unable to open file y_b.txt" << std::endl;
    }
}

// 保存 EncCuckooHashTable 到文件
void saveEncCuckooHashTableToFile(const EncCuckooHashTable& enc_cuckoo, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return;
    }

    // 保存 n 和 k
    file << enc_cuckoo.n << " " << enc_cuckoo.k << std::endl;

    // 保存 table 元素
    for (const auto& element : enc_cuckoo.table) {
        file << element << std::endl;
    }

    // 保存 stash 元素
    file << enc_cuckoo.stash.size() << std::endl; // 先保存 stash 的大小
    for (const auto& element : enc_cuckoo.stash) {
        file << element << std::endl;
    }

    file.close();
}

// 保存 valueTable 到文件
void saveValueTableToFile(const valueTable& val_table, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return;
    }

    // 保存 n 和 k
    file << val_table.n << " " << val_table.k << std::endl;

    // 保存 table 元素
    for (const auto& row : val_table.table) {
        for (const auto& vec : row) {
            file << vectorToString(vec) << " ";
        }
        file << std::endl;
    }

    // 保存 stash 元素
    file << val_table.stash.size() << std::endl; // 先保存 stash 的大小
    for (const auto& vec_vec : val_table.stash) {
        for (const auto& vec : vec_vec) {
            file << vectorToString(vec) << " ";
        }
        file << std::endl;
    }

    file.close();
}