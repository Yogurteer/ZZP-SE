#include <stdio.h>
#include <gmp.h>            // 用于大数运算
#include <fstream>          // 用于文件操作
#include <iostream>         // 用于输出
#include <gmp.h>            // 用于大数运算
#include <cmath>            // 包含 cmath 库，用于浮点运算
#include <openssl/sha.h>    // 用于SHA-256哈希
#include <iomanip>          // 用于十六进制输出
#include <chrono>           // 引入 chrono 库,用于计时
#include <set>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip> // 用于控制输出格式
#include <sstream> // 用于 stringstream
#include <cstring> // 用于处理 char*
#include <openssl/aes.h> // 使用 OpenSSL 库中的 AES 解密函数
#include "../utils/utils.h"
#include "LPSI.h"

using namespace std;

// receiver step1:求h(y)^b
void receiver_step1(mpz_t y_b, const mpz_t p, const mpz_t y, const mpz_t b) {
    // 使用 GMP 提供的 mpz_powm 函数计算 y^b mod p
    mpz_powm(y_b, y, b, p);

    // 将 y_b 的十六进制格式写入文件 y_b.txt
    FILE *outFile = fopen("dataset/OPRF/y_b.txt", "w"); // 打开文件
    if (outFile != nullptr) {
        // fprintf(outFile, "y_b in hex: ");
        mpz_out_str(outFile, 16, y_b);  // 以十六进制写入文件
        fprintf(outFile, "\n");
        fclose(outFile);  // 关闭文件
    } else {
        std::cerr << "Unable to open file y_b.txt" << std::endl;
    }
}

// receiver step2:求h(y)^bab_1
void receiver_step2(mpz_t result, const mpz_t p, mpz_t y_ba, const mpz_t b_inv) {
    // 从 y_ba.txt 读取十六进制数赋值给 y_ba
    std::ifstream inFile("dataset/OPRF/y_ba.txt");
    if (!inFile) {
        std::cerr << "Unable to open file y_ba.txt" << std::endl;
        return;
    }

    // 读取文件中的十六进制数
    std::string hexString;
    std::getline(inFile, hexString); // 读取第一行
    inFile.close(); // 关闭文件

    // 从读取的十六进制字符串转换为 mpz_t
    mpz_init(y_ba); // 确保 y_ba 已初始化
    if (mpz_set_str(y_ba, hexString.c_str(), 16) != 0) {
        std::cerr << "Failed to convert hex string to mpz_t" << std::endl;
        mpz_clear(y_ba); // 清理 mpz_t
        return;
    }

    mpz_powm(result, y_ba, b_inv, p);
}

// 从文件读取 EncCuckooHashTable
EncCuckooHashTable loadEncCuckooHashTableFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return EncCuckooHashTable(0, 0);
    }

    size_t n, k;
    file >> n >> k;
    EncCuckooHashTable enc_cuckoo(n, k);

    // 读取 table 元素
    for (size_t i = 0; i < n * k; ++i) {
        std::string element;
        file >> element;
        enc_cuckoo.table[i] = element;
    }

    // 读取 stash 元素
    size_t stash_size;
    file >> stash_size;
    enc_cuckoo.stash.resize(stash_size);
    for (size_t i = 0; i < stash_size; ++i) {
        std::string element;
        file >> element;
        enc_cuckoo.stash[i] = element;
    }

    file.close();
    return enc_cuckoo;
}

// 从文件读取 valueTable
valueTable loadValueTableFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return valueTable(0, 0, 0);
    }

    size_t n, k, stash_size;
    file >> n >> k;
    stash_size = 200;
    valueTable val_table(n, k, stash_size);

    // 读取 table 元素
    for (size_t i = 0; i < n * k; ++i) {
        std::vector<std::vector<unsigned char>> row;
        for (size_t j = 0; j < k; ++j) {
            std::string hex_str;
            file >> hex_str;
            row.push_back(stringToVector(hex_str));
        }
        val_table.table[i] = row;
    }

    // 读取 stash 元素
    file >> stash_size;
    val_table.stash.resize(stash_size);
    for (size_t i = 0; i < stash_size; ++i) {
        std::vector<std::vector<unsigned char>> row;
        for (size_t j = 0; j < k; ++j) {
            std::string hex_str;
            file >> hex_str;
            row.push_back(stringToVector(hex_str));
        }
        val_table.stash[i] = row;
    }

    file.close();
    return val_table;
}

// 子函数：匹配加密值并解密相应的索引集合并保存结果到文件
void matchAndDecrypt(
    mpz_t& r_outputprf, 
    EncCuckooHashTable& rget_encc, 
    valueTable& rget_vt, 
    mpz_t& ksym, 
    const std::string& filename) {
    // 1. 将 mpz_t r.outputprf 转换为 16 进制字符串
    std::string hex_r_outputprf = mpzToHexString(r_outputprf);
    // std::cout << "Converted r.outputprf to hex: " << hex_r_outputprf << std::endl;

    // 2. 在 rget_encc.table 中查找与 hex_r_outputprf 匹配的元素
    int pos = -1;
    bool found_in_table = false;

    for (size_t i = 0; i < rget_encc.table.size(); ++i) {
        if (rget_encc.table[i] == hex_r_outputprf) {
            pos = i;
            found_in_table = true;
            // cout << "hex_r_outputprf: " << hex_r_outputprf << endl;
            // cout << "response enc_keyword: " << rget_encc.table[i] << endl;
            // std::cout << "Match found in table at position: " << std::dec << pos << std::endl;
            break;
        }
    }

    // 3. 如果在 table 中没有找到，继续在 stash 中查找
    if (!found_in_table) {
        for (size_t i = 0; i < rget_encc.stash.size(); ++i) {
            if (rget_encc.stash[i] == hex_r_outputprf) {
                pos = i;
                std::cout << "Match found in stash at position: " <<  std::dec << pos << std::endl;
                break;
            }
        }

        // 如果在 stash 中也找不到，输出错误信息
        if (pos == -1) {
            std::cerr << "No matching element found in both table and stash!" << std::endl;
            return;
        }
    }

    // 4. 获取加密索引集合
    const std::vector<std::vector<unsigned char>>* encrypted_indices;
    if (found_in_table) {
        // 从 table 中获取加密索引集合
        encrypted_indices = &rget_vt.table[pos];
    } else {
        // 从 stash 中获取加密索引集合
        encrypted_indices = &rget_vt.stash[pos];
    }

    // 5. 对每个加密索引进行 AES 解密
    std::vector<std::string> query_result;
    int count = 0;
    for (const auto& ciphertext : *encrypted_indices) {
        count++;
        // 转换key类型
        string s_ksym = mpzToString(ksym);
        std::vector<unsigned char> v_ksym = hexstringToVector(s_ksym);

        std::vector<unsigned char> decrypted_plaintext = aes_decrypt(ciphertext, v_ksym);

        std::string decrypted_str(decrypted_plaintext.begin(), decrypted_plaintext.end());
        // 将解密结果保存到 query_result
        query_result.push_back(decrypted_str);
    }

    // 7. 将 query_result 保存到文件中
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    for (const auto& result : query_result) {
        outfile << result << std::endl;
    }
    outfile.close();

    std::cout << "Query result saved to file: " << filename << std::endl;
}

// 子函数：多关键字查询情景下:匹配加密值并解密相应的索引集合-LSE版本
void matchAndDecrypt_set(
    std::vector<mpz_t>& output_prf_set, 
    EncCuckooHashTable& rget_encc, 
    valueTable& rget_vt, 
    std::vector<mpz_t>& ksym_set, 
    vector<vector<vector<unsigned char>>> &all_query_results,  // 输出参数
    std::vector<std::vector<unsigned char>>& s_y_set) {
    
    size_t table_size = rget_encc.table.size();
    size_t element_size = rget_encc.n;
    
    // 初始化结果集合
    all_query_results.resize(output_prf_set.size());

    // 对 output_prf_set 中的每一个元素执行查询匹配和解密
    for (size_t idx = 0; idx < output_prf_set.size(); ++idx) {
        vector<unsigned char> plain_keyword = s_y_set[idx];
        mpz_t& r_outputprf = output_prf_set[idx];
        mpz_t& ksym = ksym_set[idx];

        // 1. 将 mpz_t r.outputprf 转换为 16 进制字符串
        // gmp_printf("r.output_prf为：%Zx\n", r_outputprf);
        std::string hex_r_outputprf = mpzToHexString(r_outputprf);

        // 2. 在 rget_encc.table 中查找与 hex_r_outputprf 匹配的元素
        int pos = -1;
        bool found_in_table = false;

        for (size_t i = 0; i < rget_encc.table.size(); ++i) {
            if (
                rget_encc.table[i] == hex_r_outputprf && 
                (i == hash1(plain_keyword) % (table_size) || i == hash2(plain_keyword, element_size) % (table_size))
                ) {
                pos = i;
                found_in_table = true;
                break;
            }
        }

        // 3. 如果在 table 中没有找到，继续在 stash 中查找
        if (!found_in_table) {
            for (size_t i = 0; i < rget_encc.stash.size(); ++i) {
                if (rget_encc.stash[i] == hex_r_outputprf) {
                    pos = i;
                    break;
                }
            }

            // 如果在 stash 中也找不到，输出错误信息
            if (pos == -1) {
                // 打印找不到的位置
                // std::cerr << "元素匹配失败,在sender输入集的索引为: " << idx << std::endl;
                // std::cout << "对应的 Enc_keyword: " << hex_r_outputprf << std::endl;
                // continue;
            }
        }

        if(pos == -1) {
            vector<vector<unsigned char>> query_result;
            vector<unsigned char> vec_zero(5, '0'); // 填充长度为5的零向量作为LPSI的返回值
            query_result.push_back(vec_zero);
            all_query_results[idx] = query_result;
            continue;
        }

        // 4. 获取加密索引集合
        const std::vector<std::vector<unsigned char>>* encrypted_indices;
        if (found_in_table) {
            // 从 table 中获取加密索引集合
            encrypted_indices = &rget_vt.table[pos];
        } else {
            // 从 stash 中获取加密索引集合
            encrypted_indices = &rget_vt.stash[pos];
        }

        // 5. 对每个加密索引进行 AES 解密
        vector<vector<unsigned char>> query_result;
        for (const auto& ciphertext : *encrypted_indices) {
            // 转换key类型
            std::string s_ksym = mpzToString(ksym);
            std::vector<unsigned char> v_ksym = hexstringToVector(s_ksym);

            // 解密
            vector<unsigned char> decrypted_plaintext = aes_decrypt(ciphertext, v_ksym);
            
            // 将解密结果保存到 query_result
            query_result.push_back(decrypted_plaintext);
        }

        // 将当前查询结果保存到 all_query_results 中
        all_query_results[idx] = query_result;
    }
}