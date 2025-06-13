#include "encrypt.h"

unsigned int hmac_digest(const unsigned char *plaintext, size_t plaintext_len,
    const unsigned char *key, size_t key_len,
    unsigned char *digest) {
    HMAC_CTX *ctx;

    unsigned int len;

    /* Create and initialise the context */
    ctx = HMAC_CTX_new();

    /* Initialise the decryption operation. */
    HMAC_Init_ex(ctx, key, key_len, EVP_sm3(), NULL);

    /* compute the digest */
    HMAC_Update(ctx, plaintext, plaintext_len);

    /* Finalise the digest */
    HMAC_Final(ctx, digest, &len);

    /* Clean up */
    HMAC_CTX_free(ctx);

    return len;
}

vector<unsigned char> gm_hmac(const unsigned char *data, size_t data_len,
                              const unsigned char *key, size_t key_len,size_t len)
{
    vector<unsigned char> out(EVP_MAX_MD_SIZE); 

    unsigned int actual_len = hmac_digest(data,data_len,key,key_len,out.data());

    // 截取指定长度的 HMAC
    if (len > actual_len) {
        len = actual_len; // 避免超过实际 HMAC 长度
    }
    out.resize(len);

    return out;
}

std::vector<unsigned char> sha256_hmac(const unsigned char *data, size_t data_len,
                                       const unsigned char *key, size_t key_len,size_t len) {
    if (len > EVP_MAX_MD_SIZE) {
        throw std::invalid_argument("Requested length exceeds SHA-256 HMAC size (32 bytes)");
    }

    std::vector<unsigned char> full_result(EVP_MAX_MD_SIZE); // 存储完整的 32 字节 HMAC 结果
    unsigned int md_len = 0; // 存储实际 HMAC 长度

    // 计算 HMAC-SHA256
    if (!HMAC(EVP_sha256(), key, key_len, data, data_len, full_result.data(), &md_len)) {
        return {}; // 计算失败，返回空向量
    }

    // 截取前 len 字节
    return std::vector<unsigned char>(full_result.begin(), full_result.begin() + len);
}

std::vector<unsigned char> my_xor(const std::vector<unsigned char>& data,
                               const std::vector<unsigned char>& key) {
    size_t len = data.size();
    size_t key_len = key.size();

    std::vector<unsigned char> result(len);

    for (size_t i = 0; i < len; i++) {
        result[i] = data[i] ^ key[i % key_len];// 密钥循环使用
    }

    return result;
}
