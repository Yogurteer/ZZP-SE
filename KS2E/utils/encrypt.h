#pragma once

#include "utils.h"

#include <vector>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <gmssl/sm4.h>
#include <gmssl/sm3.h>

#define AES_BLOCK_SIZE 16
#define DIGEST_SIZE 32
#define GGM_SIZE 65536
#define HASH_SIZE 3

using namespace std;


/**
 * @brief gm_hmac底层hmac实现
 * 
 * @param plaintext 
 * @param plaintext_len 
 * @param key 
 * @param key_len 
 * @param digest 
 * @return unsigned int 
 */
unsigned int hmac_digest(const unsigned char *plaintext, size_t plaintext_len,
                         const unsigned char *key, size_t key_len,unsigned char *digest);

/**
 * @brief GMSM3 哈希函数实现的Hmac
 * 
 * @param data 
 * @param data_len 
 * @param key 
 * @param key_len 
 * @param len 
 * @return vector<unsigned char> 
 */
vector<unsigned char> gm_hmac(const unsigned char *data, size_t data_len,
                              const unsigned char *key, size_t key_len,size_t len);

/**
 * @brief 计算 HMAC-SHA256 值，并截取指定长度
 * 
 * @param data 输入数据
 * @param data_len 数据长度
 * @param key 密钥
 * @param key_len 密钥长度
 * @param len 需要返回的 HMAC 结果长度（最大 32 字节）
 * @return std::vector<unsigned char> 返回 `len` 字节的 HMAC 结果
 */
std::vector<unsigned char> sha256_hmac(const unsigned char *data, size_t data_len,
                                       const unsigned char *key, size_t key_len,size_t len);

/**
 * @brief xor，输出长度等于数据长度而不是密钥长度
 * 
 * @param data
 * @param key 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> my_xor(const std::vector<unsigned char>& data,
                               const std::vector<unsigned char>& key);