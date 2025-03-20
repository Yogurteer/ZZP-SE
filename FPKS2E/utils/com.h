#ifndef COM_H
#define COM_H

#include "utils.h"

/**
 * @brief Token写入文件
 * 
 * @param Token 
 * @param Token_dir 
 */
void write_Token(const vector<vector<unsigned char>>& Token, const string& Token_dir);

/**
 * @brief 从文件读取Token 
 * 
 * @param Token_dir 
 * @return vector<vector<unsigned char>> 
 */
vector<vector<unsigned char>> read_Token(const string& Token_dir);

#endif // COM_H