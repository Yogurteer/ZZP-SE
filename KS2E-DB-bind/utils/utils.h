#pragma once

#include <iostream>  // 用于标准输入输出（cerr）
#include <iostream>
#include <fstream>   // 用于文件流操作（ifstream）
#include <sstream>
#include <string>    // 用于字符串处理（string）
#include <vector>    // 用于存储索引的动态数组（vector）
#include <unordered_map> // 用于索引的哈希表（unordered_map）
#include <map>       // 用于关键字和索引的映射（map）
#include <set>
#include <string.h>
#include <gmp.h>     // 用于大数运算
#include <chrono> // 包含chrono库用于测量时间
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>

#include "../include/DBOGaccess.hpp"
#include "encrypt.h"

using namespace std;

const unsigned char DELIMITER = ' '; // 空格可换成 0x1F 等不可见字符

std::string vectorToHexStr(const std::vector<uint8_t>& datavec);

vector<unsigned char> encodeWithDelimiter(const vector<vector<unsigned char>>& input);

vector<vector<unsigned char>> decodeWithDelimiter(const vector<unsigned char>& input);

void printVector(const std::vector<uint8_t>& datavec);

void view_pt(const std::vector<unsigned char>& v_w);

void view_vc(const std::vector<unsigned char>& vc);

std::vector<unsigned char> uint2vc(unsigned int value);

unsigned int vc2uint(const std::vector<unsigned char>& vec);

template<typename... Vectors>
std::vector<unsigned char> concatVectors(const std::vector<unsigned char>& first, const Vectors&... rest) {
    std::vector<unsigned char> result;
    result.reserve(first.size() + (rest.size() + ...));
    result.insert(result.end(), first.begin(), first.end());
    (result.insert(result.end(), rest.begin(), rest.end()), ...);
    return result;
}

/**
 * @brief Get the Current Time object
 * 
 * @return std::chrono::high_resolution_clock::time_point 
 */
std::chrono::high_resolution_clock::time_point getCurrentTime();

double getDurationSeconds(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point end);

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
 * @brief 
 * 
 * @param px 
 * @param len 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> toByteVector(const unsigned char* px, size_t len);

/**
 * @brief uint->Bytes
 * 
 * @param id 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> uintToBytes(unsigned int id);

/**
 * @brief read keywords from file
 * 
 * @param dir 
 * @param keywordMap 
 */
void read_keywords(const char* dir, map<string, vector<string>>& keywordMap);

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

void search_output_vk(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, 
                   const std::string& search_output_dir, DBOGaccess& db1);

void search_output_kv(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, 
                   const std::string& search_output_dir, DBOGaccess& db1);

/**
 * @brief write search result to files
 * 
 * @param search_result 
 * @param search_output_dir 
 */
void search_output(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, 
    const std::string& search_output_dir);
