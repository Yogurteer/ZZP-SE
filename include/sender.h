#ifndef SENDER_H
#define SENDER_H

#include <gmp.h>            // 用于大数运算
#include "LPSI.h"

struct data_sender_OPRF {
    mpz_t p;
    mpz_t kprf;
    mpz_t ksym;
};

// sender step1: 求 h(y)^(b*a)
void sender_step1(mpz_t y_ba, const mpz_t p, mpz_t y_b, const mpz_t a);

void sender_initial(mpz_t a);

// 保存 EncCuckooHashTable 到文件
void saveEncCuckooHashTableToFile(const EncCuckooHashTable& enc_cuckoo, const std::string& filename);

// 保存 valueTable 到文件
void saveValueTableToFile(const valueTable& val_table, const std::string& filename);

#endif // SENDER_H
