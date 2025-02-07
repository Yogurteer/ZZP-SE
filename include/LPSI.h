#ifndef SINGLE_LPSI_H
#define SINGLE_LPSI_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <gmp.h> // 如果使用 GMP 库用于大整数处理
#include <cstdlib> // for rand()
#include <ctime>   // for time()
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace std;

struct CuckooHashTable {
    size_t n; // 原始元素数量
    size_t k; // 哈希函数数量
    vector<vector<unsigned char>> table; // 哈希表
    vector<vector<unsigned char>> stash; // stash 用于存储无法插入的元素
    static const size_t maxIterations = 20; // 最大迭代次数

    CuckooHashTable(size_t n, size_t k) : n(n), k(k), table(n * k) {
        srand(time(nullptr)); // 初始化随机种子
    }
};

// 定义新的结构体 EncCuckooHashTable，其 table 和 stash 的元素类型为 string
struct EncCuckooHashTable {
    size_t n; // 原始元素数量
    size_t k; // 哈希函数数量
    vector<string> table; // 哈希表
    vector<string> stash; // stash 用于存储无法插入的元素
    static const size_t maxIterations = 20; // 最大迭代次数

    EncCuckooHashTable(size_t n, size_t k) : n(n), k(k), table(n * k) {
    }
};

struct valueTable {
    size_t n; // 原始元素数量
    size_t k; // 哈希函数数量
    std::vector<std::vector<std::vector<unsigned char>>> table; // 哈希表
    std::vector<std::vector<std::vector<unsigned char>>> stash; // stash 用于存储无法插入的元素
    static const size_t maxIterations = 20; // 最大迭代次数

    valueTable(size_t n, size_t k, size_t len_stash) : n(n), k(k), table(n * k), stash(len_stash) {
        srand(time(nullptr)); // 初始化随机种子
    }
};

size_t hash1(const std::vector<unsigned char>& key);

size_t hash2(const std::vector<unsigned char>& key, size_t n);

void insert(CuckooHashTable &hashTable, std::string key);

void fillEmptyPositionsWithRandomNumbers(CuckooHashTable &hashTable);

void printTable(const CuckooHashTable &hashTable);

void printStash(const CuckooHashTable &hashTable);

// 优化后:根据cuckootable填充到valuetable,并对每个索引进行对称加密 LSE版本
void populateValueTable(map<vector<unsigned char>, vector<vector<unsigned char>>> enc_keywordMap, 
                         const CuckooHashTable& cuckoo, 
                         valueTable& value, mpz_t p, mpz_t key);

// cuckoo 插入
void insert(CuckooHashTable &hashTable, vector<unsigned char> key);

bool find(const CuckooHashTable &hashTable, vector<unsigned char> key);

// LSE中插入所有关键字
void insert_all(map<vector<unsigned char>, vector<vector<unsigned char>>> enc_keywordMap, CuckooHashTable& cuckoo);

void viewdata_cuckoo_value(CuckooHashTable& cuckoo, valueTable& valuetable);

void viewlen_cuckoo_value(CuckooHashTable& cuckoo, valueTable& valuetable);

// 对 cuckoo 中的元素进行加密并插入到 enc_cuckoo 中的子函数
void encryptCuckooHashTable(const CuckooHashTable &cuckoo, EncCuckooHashTable &enc_cuckoo);

// 处理 SToken 数据，右值转化为字节向量
void convertSToken(std::map<std::string, std::vector<std::vector<unsigned char>>>& result,
                   const std::map<std::string, std::vector<unsigned int>>& SToken);

// 单点LPSI协议
void single_LPSI(map<string,vector<unsigned int>> SToken, 
                 string query_w, 
                 std::vector<std::vector<unsigned int>> &all_query_results);

// 将EDB转化为字节向量键值对
void convert_EDB(map<vector<unsigned char>, vector<unsigned char>>& EDB,
                 std::map<std::vector<unsigned char>, std::vector<std::vector<unsigned char>>>& result);

// 多点LPSI
void multi_LPSI(int yk_size,int xk_size,
                map<vector<unsigned char>, vector<vector<unsigned char>>> EDB, 
                vector<vector<unsigned char>> Tokenlist, 
                map<vector<unsigned char>, vector<vector<unsigned char>>> &multi_LPSI_results);

#endif // SINGLE_LPSI_H
