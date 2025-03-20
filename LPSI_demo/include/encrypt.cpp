// 融合aes_cbc.cpp hmac.cpp OPRF.cpp PRF.cpp XOR.cpp

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "../utils/utils.h"
#include "encrypt.h"
#include "sender.h"
#include "receiver.h"

using namespace std;

// 固定不变的IV（16字节）
const std::vector<unsigned char> FIXED_IV = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};

// AES CBC 加密函数,输入为字节向量
std::vector<unsigned char> aes_encrypt(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& key) {
    // 清理错误栈
    ERR_clear_error();

    // 创建加密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context");
    }

    std::vector<unsigned char> ciphertext;
    int len;
    int ciphertext_len;

    // 生成随机IV
    std::vector<unsigned char> iv(AES_BLOCK_SIZE);
    if (!RAND_bytes(iv.data(), AES_BLOCK_SIZE)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("IV generation failed");
    }

    // 初始化加密操作
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption initialization failed");
    }

    // 加密操作
    int plaintext_len = plaintext.size();
    ciphertext.resize(plaintext_len + AES_BLOCK_SIZE);  // 预留足够的空间

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }
    ciphertext_len = len;

    // 完成加密
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Final encryption step failed");
    }
    ciphertext_len += len;

    // 释放上下文
    EVP_CIPHER_CTX_free(ctx);

    // 在返回之前，将IV附加在密文的前面
    ciphertext.insert(ciphertext.begin(), iv.begin(), iv.end());
    ciphertext.resize(ciphertext_len + AES_BLOCK_SIZE);  // 调整到实际大小

    return ciphertext;
}

// AES CBC 解密函数
std::vector<unsigned char> aes_decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key) {
    // 清理错误栈
    ERR_clear_error();

    // 创建解密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create decryption context");
    }

    int len;
    int plaintext_len;
    std::vector<unsigned char> plaintext(ciphertext.size());

    // 提取IV
    if (ciphertext.size() < AES_BLOCK_SIZE) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Ciphertext too short");
    }
    std::vector<unsigned char> iv(ciphertext.begin(), ciphertext.begin() + AES_BLOCK_SIZE);

    // 初始化解密操作
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption initialization failed");
    }

    // 解密操作
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data() + AES_BLOCK_SIZE, ciphertext.size() - AES_BLOCK_SIZE) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ERR_print_errors_fp(stderr);  // 输出OpenSSL错误
        throw std::runtime_error("Decryption failed");
    }
    plaintext_len = len;

    // 完成解密
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Final decryption step failed");
    }
    plaintext_len += len;

    // 释放上下文
    EVP_CIPHER_CTX_free(ctx);

    // 调整明文大小
    plaintext.resize(plaintext_len);

    return plaintext;
}

// 字节向量sha256_hmac
std::vector<unsigned char> sha256_hmac(const char *data, const char *key, size_t len) {
    unsigned char *hmac_result;

    // 创建 HMAC_SHA256 计算对象
    hmac_result = HMAC(EVP_sha256(), key, strlen(key), (unsigned char *)data, strlen(data), NULL, NULL);
    
    if (hmac_result == NULL) {
        return {}; // 如果hmac_result为空，返回空vector
    }

    // 创建一个字节向量来存储结果，大小为len
    std::vector<unsigned char> result(hmac_result, hmac_result + len);

    return result;
}

// OPRF实现
void OPRF(mpz_t key, mpz_t y, mpz_t p, mpz_t b, mpz_t b_inv, mpz_t output) {
    mpz_t y_b,y_ba,y_bab_1;
    mpz_init(y_b);
    mpz_init(y_ba);
    mpz_init(y_bab_1);

    //---sender和receiver共同执行协议----//

    // 1. receiver计算 y^b % p
    receiver_step1(y_b, p, y, b);

    // 2. sender计算 (y^b)^a % p = y^(b*a) % p
    sender_step1(y_ba, p, y_b, key);

    // 3. 计算 (y^(b*a))^(b-1) % p = y^a % p,得到result
    receiver_step2(y_bab_1, p, y_ba, b_inv);

    truncate_to_1024bit(output, y_bab_1);

    // 清理 GMP 变量
    mpz_clear(y_b);
    mpz_clear(y_ba);
    mpz_clear(y_bab_1);

    return ;
}

// PRF伪随机函数实现
void F_k_x(mpz_t result, std::vector<unsigned char> s_x, const mpz_t p, const mpz_t ksym) {
    // Step 1: Compute SHA256 of the input vector `s_x`
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    // `s_x.data()` 返回指向字节向量的指针, `s_x.size()` 返回字节向量的大小
    SHA256_Update(&sha256, s_x.data(), s_x.size());
    SHA256_Final(hash, &sha256);

    // Step 2: Truncate the hash to 32 bits
    unsigned int hx = 0;
    for (int i = 0; i < 4; ++i) {
        hx = (hx << 8) | hash[i];
    }

    // Step 3: Convert hx to mpz_t for further computation
    mpz_t hx_mpz;
    mpz_init_set_ui(hx_mpz, hx);

    // Step 4: Compute `result = hx^ksym mod p`
    mpz_powm(result, hx_mpz, ksym, p);

    // Step 5: Truncate `result` to 1024 bits
    mpz_t result_1024bit;
    mpz_init(result_1024bit);
    truncate_to_1024bit(result_1024bit, result);
    mpz_set(result, result_1024bit); // result作为函数输出结果

    // Step 6: Clear temporary variables
    mpz_clear(hx_mpz);
    mpz_clear(result_1024bit);
}

// XOR 加密函数-字符串
std::vector<unsigned char> xor_encrypt(const std::vector<unsigned char>& plaintext,
                                           const std::vector<unsigned char>& key) {
    size_t len = plaintext.size();
    size_t key_len = key.size();

    std::vector<unsigned char> ciphertext(len);  // 创建一个与明文相同长度的 vector 用于存储密文

    for (size_t i = 0; i < len; i++) {
        ciphertext[i] = plaintext[i] ^ key[i % key_len];  // 对明文和密钥进行异或操作，密钥循环使用
    }

    return ciphertext;
}

// XOR 解密函数-字符串
std::vector<unsigned char> xor_decrypt(const std::vector<unsigned char>& ciphertext,
                                           const std::vector<unsigned char>& key) {
    size_t len = ciphertext.size();
    size_t key_len = key.size();

    std::vector<unsigned char> plaintext(len);  // 创建一个与密文相同长度的 vector 用于存储明文

    for (size_t i = 0; i < len; i++) {
        plaintext[i] = ciphertext[i] ^ key[i % key_len];  // 对密文和密钥进行异或操作，密钥循环使用
    }

    return plaintext;
}