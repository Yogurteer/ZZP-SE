#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <vector>
#include <string>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <gmp.h>
#include <stdio.h>
#include <stdint.h>  // 使用 uint32_t 类型


// AES CBC 加密函数,输入为字节向量
std::vector<unsigned char> aes_encrypt(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& key);

// AES CBC 解密函数声明
std::vector<unsigned char> aes_decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key);

// 字节数组sha256_hmac
unsigned char* sha256_hmac_len(const char *data, const char *key, size_t len);

// 字节向量sha256_hmac
std::vector<unsigned char> sha256_hmac(const char *data, const char *key, size_t len);

// OPRF实现
void OPRF(mpz_t key, mpz_t y, mpz_t p, mpz_t b, mpz_t b_inv, mpz_t output);

// PRF伪随机函数实现
void F_k_x(mpz_t result, std::vector<unsigned char> s_x, const mpz_t p, const mpz_t ksym);

// XOR 加密函数-字符串
std::vector<unsigned char> xor_encrypt(const std::vector<unsigned char>& plaintext,
                                           const std::vector<unsigned char>& key);

// XOR 解密函数-字符串
std::vector<unsigned char> xor_decrypt(const std::vector<unsigned char>& ciphertext,
                                           const std::vector<unsigned char>& key);

#endif // ENCRYPT_H