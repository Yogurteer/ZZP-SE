#ifndef RECEIVER_H
#define RECEIVER_H

#include <gmp.h>            // 用于大数运算
#include <stdio.h>
#include <iostream>
#include <iomanip>          // 用于十六进制输出
#include "LPSI.h"

#define shorted_length   4 // h(y)截断字节数
#define out_length       4 // OPRF协议输出的截断字节数,如h(y)^a和h(y)^bab_1

using namespace std;

struct data_receiver_OPRF {
    mpz_t p;
    std::vector<mpz_t> y_set;
    std::vector<mpz_t> output_sym_set;
    std::vector<mpz_t> output_prf_set;
    std::vector<mpz_t> b_set;
    std::vector<mpz_t> b_1_set;

    // 构造函数，接收参数 n 用于初始化 y_set, output_sym_set, output_prf_set, b_set, b_1_set
    data_receiver_OPRF(size_t n) : y_set(n), output_sym_set(n), output_prf_set(n), b_set(n), b_1_set(n) {
        mpz_init(p);

        // 初始化 y_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < n; ++i) {
            mpz_init(y_set[i]);
        }

        // 初始化 output_sym_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < n; ++i) {
            mpz_init(output_sym_set[i]);
        }

        // 初始化 output_prf_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < n; ++i) {
            mpz_init(output_prf_set[i]);
        }

        // 初始化 b_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < n; ++i) {
            mpz_init(b_set[i]);
        }

        // 初始化 b_1_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < n; ++i) {
            mpz_init(b_1_set[i]);
        }
    }

    // 析构函数，释放所有 mpz_t 变量的内存
    ~data_receiver_OPRF() {
        mpz_clear(p);
        
        // 清理 y_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < y_set.size(); ++i) {
            mpz_clear(y_set[i]);
        }

        // 清理 output_sym_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < output_sym_set.size(); ++i) {
            mpz_clear(output_sym_set[i]);
        }

        // 清理 output_prf_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < output_prf_set.size(); ++i) {
            mpz_clear(output_prf_set[i]);
        }

        // 清理 b_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < b_set.size(); ++i) {
            mpz_clear(b_set[i]);
        }

        // 清理 b_1_set 中的每个 mpz_t 元素
        for (size_t i = 0; i < b_1_set.size(); ++i) {
            mpz_clear(b_1_set[i]);
        }
    }
};

// receiver step1: 求 h(y)^b
void receiver_step1(mpz_t y_b, const mpz_t p, const mpz_t y, const mpz_t b);

// receiver step2: 求 h(y)^bab_1
void receiver_step2(mpz_t result, const mpz_t p, mpz_t y_ba, const mpz_t b_inv);

// receiver initial
void receiver_initial(mpz_t y, mpz_t b, mpz_t b_inv);

// 从文件读取 EncCuckooHashTable
EncCuckooHashTable loadEncCuckooHashTableFromFile(const std::string& filename);

// 从字符串中还原一维 unsigned char 数组
std::vector<unsigned char> stringToVector(const std::string& str);

// 从文件读取 valueTable
valueTable loadValueTableFromFile(const std::string& filename);

// 子函数：多关键字查询情景下:匹配加密值并解密相应的索引集合-LSE版本
void matchAndDecrypt_set(
    std::vector<mpz_t>& output_prf_set, 
    EncCuckooHashTable& rget_encc, 
    valueTable& rget_vt, 
    std::vector<mpz_t>& ksym_set, 
    vector<vector<vector<unsigned char>>> &all_query_results,  // 输出参数
    std::vector<std::vector<unsigned char>>& s_y_set);

#endif // RECEIVER_H
