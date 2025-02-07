#ifndef UTILS_H
#define UTILS_H


#include <stdio.h>   // 标准输入输出
#include <iostream>  // 用于标准输入输出（cerr）
#include <fstream>   // 用于文件流操作（ifstream）
#include <string>    // 用于字符串处理（string）
#include <vector>    // 用于存储索引的动态数组（vector）
#include <map>       // 用于关键字和索引的映射（map）
#include <gmp.h>     // 用于大数运算
#include <chrono> // 包含chrono库用于测量时间
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <cmath>
#include <iomanip>
#include <string>
#include <cstring>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <gmpxx.h>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <set>

#include "LPSI.h"
#include "encrypt.h"

using namespace std;

# define shorted_length   4 //h(y)截断字节数
# define out_length   4     //OPRF协议输出的截断字节数,如h(y)^a和h(y)^bab_1
// # define table_size 10000
// # define element_size 5000

const size_t n = 5000; // 原始元素数量
const size_t c = 2;  // 碰撞常数c，table_size b=n*c 等于cuckoohash中hash函数的个数

// 封装打印 vector<vector<unsigned char>> 的函数
void printYToFile(const vector<vector<unsigned char>>& data, const string& dir, const string& title);

// 封装打印 map<vector<unsigned char>, vector<vector<unsigned char>>> 的函数
void printMapToFile(const map<vector<unsigned char>, vector<vector<unsigned char>>>& data, const string& dir, const string& title);

// 封装函数打印三维vector并将结果写入文件
void write_ciphertexts_to_file(const std::vector<std::vector<std::vector<unsigned char>>>& ciphertexts, const std::string& filename);

// 展开tokenlist
void flatten_tokenlist(const std::vector<std::vector<std::vector<unsigned char>>>& all_tokenlist,
                       std::vector<std::vector<unsigned char>>& flattened_tokenlist);

// parse tokens
void parse_tokens(const std::vector<std::vector<std::vector<unsigned char>>>& tokens, 
                  std::vector<std::vector<std::vector<unsigned char>>>& all_tokenlist, 
                  std::vector<std::vector<std::vector<unsigned char>>>& all_symlist);

// 转换查询键集合为字节向量
void convert_query_set(const std::vector<std::string>& set_query, 
                       std::vector<std::vector<unsigned char>>& vc_set_query);

// 转换SToken的键和值为字节向量
void convertmap_string2vc(map<string,vector<vector<unsigned char>>>& SToken,
                          map<vector<unsigned char>, vector<vector<unsigned char>>>& vc_SToken);

// 解析R，从字符数组转换为unit和char
std::pair<unsigned int, char> parse_R(const std::vector<unsigned char>& pt);

// 写入LDSE user获取结果传入 query_set 和 R，将它们写入到指定的文件中
void write_LDSE_R_multi(const std::vector<std::string>& query_w, vector<pair<unsigned int, char>> R, const std::string& filename, int max_w_r);

// 转换字符数组为字符串string
void convert_Tokenlist_s(const vector<unsigned char*>& all_tokenlist, vector<string>& s_all_Tokenlist, size_t length);

// 转换字节向量为字节数组
void convert_uc_vc(const vector<vector<vector<unsigned char>>>& multi_LPSI_results, vector<vector<unsigned char*>>& tokens);

// 辅助函数：将 unsigned char* 转换为 string
string uchar_ptr_to_string(unsigned char* ptr);

// 转换函数
void convert_EDB_uc(map<unsigned char *, unsigned char *> &EDB, map<string, vector<unsigned char*>> &uc_EDB);

// 将SToken的所有值转化为字符数组,键仍为string
void convert_SToken(const std::map<std::string, std::vector<unsigned int>>& SToken,
                    std::map<string, std::vector<unsigned char*>>& uc_SToken);

// 拼接操作：将 unsigned int id 和 char op 拼接成 unsigned char 数组 id_op
std::vector<unsigned char> concat_id_op(unsigned int id, char op);

// 写入LDSE user获取结果传入 query_w 和 R，将它们写入到指定的文件中
void write_LDSE_R_single(const std::string& query_w, const std::vector<unsigned int>& R, const std::string& filename);

// 随机从W中生成一个w
string getRandomElement(const set<string>& W);

// 4位字节向量->unsigned int
unsigned int vectorToUInt(const std::vector<unsigned char>& vec);

// 随机生成32bit二进制字符串表示key
void generate_random_key(char *key, size_t len);

// 将 EDB 转换为 enc_keywordmap
void convertEDBToEncKeywordMap(
    const std::map<std::string, std::vector<std::vector<unsigned char>>>& EDB,
    std::map<std::string, std::vector<std::string>>& enc_keywordmap
);

// 将 vector<unsigned char> 转换为 string
std::string vectorToString(const std::vector<unsigned char>& vec);

// 从字符串中还原一维 unsigned char 数组
std::vector<unsigned char> stringToVector(const std::string& str);

// 函数：生成一个256位的随机数并存储在mpz_t类型中
void generate_256_bit_key(mpz_t key);

// 读取文件并保存到keywordmap
void read_keywords(char* dir, map<string, vector<unsigned int>>& keywordMap);

// 从文件中读取p
void read_p_FromFile(const char* filename, mpz_t p);

// 将mpz_t input截断1024bit赋值给output
void truncate_to_1024bit(mpz_t output, const mpz_t input);

// 将 std::vector<string> s_y_set 中的每个元素求 SHA256 并截断 256bit 再转化为 mpz_t 类型，结果保存为 std::vector<mpz_t> y_set
void s_y_set_to_h_y_set(std::vector<mpz_t>& y_set, std::vector<std::vector<unsigned char>>& s_y_set);

// 从 map<string, vector<int>> 中不重复地随机选择 n 个键
std::vector<std::string> getRandomKeysFromMap(const std::map<std::string, std::vector<int>>& keywordMap, size_t n);

// 从b_pairs.txt文件中读取两个大整数赋值给mpz_t b和mpz_t b_inv
void read_b_pairs_from_file(const std::string &filename, mpz_t b, mpz_t b_inv);

// 子函数：加密 CuckooHashTable 并赋值给 EncCuckooHashTable
void encryptCuckooHashTable(
    const CuckooHashTable &cuckoo, 
    EncCuckooHashTable &enc_cuckoo, 
    const mpz_t p, 
    const mpz_t ksym);

// 将 mpz_t 转换为 16 进制字符串
std::string mpzToHexString(const mpz_t num);

// 将 CuckooHashTable 中的数据保存到文件
void saveCuckooHashTableToFile(const CuckooHashTable& hashTable);

//mpz转string
std::string mpzToString(const mpz_t value);

// string to vector
std::vector<unsigned char> hexstringToVector(const std::string& keyHex);

#endif // UTILS_H