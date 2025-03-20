#ifndef UTILS_H
#define UTILS_H

#include <iostream>  // 用于标准输入输出（cerr）
#include <fstream>   // 用于文件流操作（ifstream）
#include <sstream>
#include <string>    // 用于字符串处理（string）
#include <vector>    // 用于存储索引的动态数组（vector）
#include <map>       // 用于关键字和索引的映射（map）
#include <set>
#include <string.h>
#include <gmp.h>     // 用于大数运算
#include <chrono> // 包含chrono库用于测量时间
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>

using namespace std;

/**
 * @brief Get the Current Time object
 * 
 * @return std::chrono::high_resolution_clock::time_point 
 */
std::chrono::high_resolution_clock::time_point getCurrentTime();

/**
 * @brief merge Ulist files
 * 
 * @param file1 
 * @param file2 
 * @param outputFile 
 */
void mergeFiles(const std::string& file1, const std::string& file2, const std::string& outputFile);

/**
 * @brief gen key with length
 * 
 * @param key 
 * @param len 
 */
void generate_random_key(char *key, size_t len);

/**
 * @brief 字节数组->字节向量
 * 
 * @param px 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> toByteVector(const char* px);

/**
 * @brief uint->Bytes
 * 
 * @param id 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> uintToBytes(unsigned int id);

/**
 * @brief L_1 + L_2
 * 
 * @param L_1 
 * @param L_2 
 * @return vector<unsigned char> 
 */
vector<unsigned char> concatenate_vectors(const std::vector<unsigned char>& L_1, const std::vector<unsigned char>& L_2);

/**
 * @brief read keywords from file
 * 
 * @param dir 
 * @param keywordMap 
 */
void read_keywords(char* dir, map<string, vector<unsigned int>>& keywordMap);

/**
 * @brief Get the L_1 ks_1 of each len f_len
 * 
 * @param L_1_ks_1 
 * @param L_1 
 * @param ks_1 
 * @param f_len 
 */
void get_L_1_ks_1(std::vector<unsigned char>& L_1_ks_1,std::vector<unsigned char>& L_1,std::vector<unsigned char>& ks_1, size_t f_len);

/**
 * @brief bytes->uint
 * 
 * @param bytes 
 * @return unsigned int 
 */
unsigned int bytesToUint(const std::vector<unsigned char>& bytes);



#endif // UTILS_H