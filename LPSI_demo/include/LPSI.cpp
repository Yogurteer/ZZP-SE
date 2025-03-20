#include <gmp.h>

#include "../utils/utils.h"
#include "sender.h"
#include "receiver.h"
#include "LPSI.h"

using namespace std;

// 处理SToken,右值转化为字节向量
void convertSToken(std::map<std::string, std::vector<std::vector<unsigned char>>>& result,
                const std::map<string, std::vector<unsigned int>>& SToken) {
    // 清空 result 以避免以前的内容影响
    result.clear();

    // 遍历原始的 map<int, vector<int>>
    for (const auto& pair : SToken) {
        // Step 1: 将 int 类型的键转换为 string
        std::string str_key = pair.first;

        // Step 2: 创建一个新的 vector<vector<unsigned char>> 来存储每个整数的转换结果
        std::vector<std::vector<unsigned char>> byte_vectors;

        // Step 3: 将 vector<int> 中的每个 int 转换为 vector<unsigned char>
        for (unsigned int value : pair.second) {
            std::vector<unsigned char> byte_vector;

            // 将unsigned int 转换为 unsigned char向量（假设每个unsigned  int 是 4 字节）
            for (size_t i = 0; i < sizeof(unsigned int); ++i) {
                byte_vector.push_back(static_cast<unsigned char>((value >> (i * 8)) & 0xFF));
            }

            byte_vectors.push_back(byte_vector);
        }

        // 将转换后的键值对插入到结果 map 中
        result[str_key] = byte_vectors;
    }
}

// 将EDB转化为字节向量键值对
void convert_EDB(map<vector<unsigned char>, vector<unsigned char>>& EDB,
                 std::map<std::vector<unsigned char>, std::vector<std::vector<unsigned char>>>& result) {
    // 清空 result 以避免以前的内容影响
    result.clear();

    // 遍历 EDB 中的每个键值对
    for (const auto& pair : EDB) {
        // 获取键（转换为 vector<unsigned char>）
        std::vector<unsigned char> key = pair.first;  // 长度为5的字节数组转为vector

        // 获取值（转换为 vector<unsigned char>）
        std::vector<unsigned char> value = pair.second; // 长度为5的字节数组转为vector

        // 将值放入一个长度为 1 的 vector 中
        std::vector<std::vector<unsigned char>> converted_values = { value };

        // 将转换后的键值对插入到 result map 中
        result[key] = converted_values;
    }
}

// 多点LPSI
void multi_LPSI(int xk_size,int yk_size,
                map<vector<unsigned char>, vector<vector<unsigned char>>> EDB, 
                vector<vector<unsigned char>> Tokenlist, 
                map<vector<unsigned char>, vector<vector<unsigned char>>> &multi_LPSI_results)
{
    cout << "//------Multi LPSI协议执行------//" << endl;

    // 0.初始化需要的变量名
    // 处理输入数据集EDB
    map<vector<unsigned char>, vector<vector<unsigned char>>> enc_keywordMap = EDB;
    // 初始化labels结果
    vector<vector<vector<unsigned char>>> results_labels;

    vector<vector<unsigned char>> s_y_set = Tokenlist;
    cout << "查询关键字的数量: " << Tokenlist.size() << endl;

    int num = Tokenlist.size(); // 查找的关键字个数,等于Tokenlist长度
    mpz_t p,ksym,kprf,fkx;
    data_receiver_OPRF r(num);
    data_sender_OPRF s;
    mpz_init(s.p);
    mpz_init(s.kprf);
    mpz_init(s.ksym);
    mpz_init(r.p);

    // 初始化其他变量
    mpz_init(p);
    mpz_init(ksym);
    mpz_init(kprf);
    mpz_init(fkx);

    // 初始化s.kprf,s.ksym
    generate_256_bit_key(ksym);
    generate_256_bit_key(kprf);

    CuckooHashTable cuckoo(xk_size, c); // sender holds x_labels

    // 1.初始化

    // 初始化公钥 p
    read_p_FromFile("dataset/OPRF/p-2048.txt", p); // 从文件中读p 2048bit
    mpz_set(r.p, p);
    mpz_set(s.p, p);

    // 初始化 s.kprf 和 s.ksym 和 r.y_set
    mpz_set(s.ksym, ksym);
    mpz_set(s.kprf, kprf);
    s_y_set_to_h_y_set(r.y_set, s_y_set);

    // 2.遍历查询关键字集合, 每个关键字求2次OPRF
    for (int i = 0; i < num; i++) {
        // a. 初始化receiver 私钥 b 以及 b-1
        read_b_pairs_from_file("dataset/OPRF/b_pairs.txt", r.b_set[i], r.b_1_set[i]);

        // b. 对于receiver的y和sender，执行2次OPRF
        OPRF(s.kprf, r.y_set[i], p, r.b_set[i], r.b_1_set[i], r.output_prf_set[i]); // 一次OPRF协议需一轮通信
        OPRF(s.ksym, r.y_set[i], p, r.b_set[i], r.b_1_set[i], r.output_sym_set[i]); // 两次OPRF可以在一个round中运行
    }

    // 3.将keywordmap中的keyword全部插入cuckoo table, 对应的索引集合插入valuetable
    insert_all(enc_keywordMap, cuckoo);
    fillEmptyPositionsWithRandomNumbers(cuckoo);

    // 初始化 valuetable
    int len_stash = cuckoo.stash.size();
    valueTable valuetable(xk_size, c, len_stash);

    // 加密索引集合
    populateValueTable(enc_keywordMap, cuckoo, valuetable, p, s.ksym);

    // 4.sender将cuckoo转化为enc_cuckoo, 用KPRF对每个关键字加密
    EncCuckooHashTable enc_cuckoo(xk_size, c);
    encryptCuckooHashTable(cuckoo, enc_cuckoo, p, s.kprf);

    // 5.sender 把enc_cuckoo和valuetable存入文件传输给 receiver
    string dir_enc_cuckoo = "dataset/enc_cuckoo.txt";
    string dir_valuetable = "dataset/valuetable.txt";
    // saveEncCuckooHashTableToFile(enc_cuckoo, dir_enc_cuckoo);
    // saveValueTableToFile(valuetable, dir_valuetable);
    
    // EncCuckooHashTable rget_encc = loadEncCuckooHashTableFromFile(dir_enc_cuckoo);
    // valueTable rget_vt = loadValueTableFromFile(dir_valuetable);

    // 6.receiver 读取文件, 对每个加密关键字在enc_cuckoo中查找匹配并在valuetable中解密相应的索引集合
    matchAndDecrypt_set(r.output_prf_set, enc_cuckoo, valuetable, r.output_sym_set, results_labels, s_y_set);

    // 7.合并待查询keyword和labels
    for(size_t i=0;i<Tokenlist.size();i++){
        multi_LPSI_results[Tokenlist[i]]=results_labels[i];
    }
    cout<<"Multi LPSI Query success!"<<endl;
    cout<<endl;
}

size_t hash1(const std::vector<unsigned char>& key) {
    // 使用 std::hash 来计算 vector<unsigned char> 的哈希值
    // 将 vector<unsigned char> 转换为字节数组并计算哈希
    return std::hash<std::string>()(std::string(key.begin(), key.end()));
}

size_t hash2(const std::vector<unsigned char>& key, size_t n) {
    // 使用 hash1 来获取哈希值并进行除法处理
    return (hash1(key) / n);
}

// cuckoo 插入
void insert(CuckooHashTable &hashTable, vector<unsigned char> key) {
    size_t iter = 0;
    size_t index = hash1(key) % (hashTable.n * hashTable.k);

    while (iter < CuckooHashTable::maxIterations) {
        if (hashTable.table[index].empty()) {
            hashTable.table[index] = key;  // 如果位置为空，直接插入
            return;
        } else {
            vector<unsigned char> evictedKey = hashTable.table[index]; // 记录被踢出的元素
            hashTable.table[index] = key;  // 插入新元素

            // 重新尝试插入被踢出的元素
            key = evictedKey; 
            index = hash2(key, hashTable.n) % (hashTable.n * hashTable.k); // 使用第二个哈希函数
        }
        iter++;
    }
    // 如果达到最大迭代次数，将无法插入的元素放入stash
    hashTable.stash.push_back(key);

}

void fillEmptyPositionsWithRandomNumbers(CuckooHashTable &hashTable) {
    // 初始化随机数种子
    std::srand(std::time(0));

    for (size_t i = 0; i < hashTable.table.size(); ++i) {
        if (hashTable.table[i].empty()) {
            // 创建一个长度为 5 的字节向量，并填充随机字节
            std::vector<unsigned char> randomBytes(5);
            for (size_t j = 0; j < 5; ++j) {
                randomBytes[j] = static_cast<unsigned char>(rand() % 256);  // 生成随机字节 (0-255)
            }
            // 将随机字节向量转换为字符串存入哈希表
            hashTable.table[i] = randomBytes;
        }
    }
}

bool find(const CuckooHashTable &hashTable, vector<unsigned char> key) {
    size_t index = hash1(key) % (hashTable.n * hashTable.k);
    if (hashTable.table[index] == key) return true;

    index = hash2(key, hashTable.n) % (hashTable.n * hashTable.k);
    if (hashTable.table[index] == key) return true;

    return false;
}

void printStash(const CuckooHashTable &hashTable) {

    std::cout << "cuckoo stash len: " << hashTable.stash.size() << std::endl;
}

// 优化后:根据cuckootable填充到valuetable,并对每个索引进行对称加密 LSE版本
void populateValueTable(map<vector<unsigned char>, vector<vector<unsigned char>>> enc_keywordMap, 
                         const CuckooHashTable& cuckoo, 
                         valueTable& value, mpz_t p, mpz_t key) {
    vector<unsigned char> keyword;

    // 清空value.table 
    for (size_t i = 0; i < value.table.size(); ++i){
        value.table[i].clear();
    }

    // 遍历 table
    for (size_t i = 0; i < cuckoo.table.size(); ++i) {
        // 在cuckoo.table和value.table中的索引均为i
        keyword = cuckoo.table[i];
        mpz_t fx;
        mpz_init(fx);
        F_k_x(fx, keyword, p, key);
        string s_ksym = mpzToString(fx);
        std::vector<unsigned char> v_ksym = hexstringToVector(s_ksym);
        mpz_clear(fx);

        const std::vector<std::vector<unsigned char>>& indices = enc_keywordMap[keyword];

        // 存wi对应的ids的加密集合enc_indices
        std::vector<std::vector<unsigned char>> enc_indices(indices.size());

        for (size_t j = 0; j < indices.size(); ++j) {
            vector<unsigned char> index_v = indices[j];

            std::vector <unsigned char> ciphertext = aes_encrypt(index_v, v_ksym);
            
            enc_indices[j]=ciphertext;

            string s_ciphertext = vectorToString(ciphertext);
        }
        // 插入value.table
        value.table[i] = enc_indices;
    }

    // 遍历 stash
    for (size_t i = 0; i < cuckoo.stash.size(); ++i) {
        // 在cuckoo.stash和value.stash中的索引均为i
        keyword = cuckoo.stash[i];
        mpz_t fx;
        mpz_init(fx);
        F_k_x(fx, keyword, p, key);
        string s_ksym = mpzToString(fx);
        std::vector<unsigned char> v_ksym = hexstringToVector(s_ksym);
        mpz_clear(fx);

        const std::vector<std::vector<unsigned char>>& indices = enc_keywordMap[keyword];
        
        // 存wi对应的ids的加密集合enc_indices
        std::vector<std::vector<unsigned char>> enc_indices(indices.size());

        for (size_t j = 0; j < indices.size(); ++j) {
            vector<unsigned char> index_v = indices[j]; 

            std::vector <unsigned char> ciphertext = aes_encrypt(index_v, v_ksym);
            
            enc_indices[j]=ciphertext;

            string s_ciphertext = vectorToString(ciphertext);
        }
        // 插入value.table
        value.stash[i] = enc_indices;
    }
}

// 在cuckoo table中插入所有关键字
void insert_all(map<vector<unsigned char>, vector<vector<unsigned char>>> enc_keywordMap, CuckooHashTable& cuckoo){
    // 遍历keywordMap的所有键，将每个键插入到Cuckoo中
    for (const auto& pair : enc_keywordMap) {
        vector<unsigned char> key = pair.first; // 获取当前键
        insert(cuckoo, key);
    }
}

void viewlen_cuckoo_value(CuckooHashTable& cuckoo, valueTable& valuetable){
    cout<<"cuckoo table len: "<<cuckoo.table.size()<<endl;
    cout<<"cuckoo stash len: "<<cuckoo.stash.size()<<endl;
    cout<<"valuetable table len: "<<valuetable.table.size()<<endl;
    cout<<"valuetable stash len: "<<valuetable.stash.size()<<endl;
}

void single_find(map<string, vector<int>> keywordMap, string s){
    // 测试查找某个关键字对应的索引集合
    auto it = keywordMap.find(s);
    int loc = 0; // 表示键为s的元素的下标
    for (auto iter = keywordMap.begin(); iter != it; ++iter) {
        ++loc;
    }
    cout<<"Location of "<<s<<": "<<loc<<endl;
    // 打印该关键字下的值
    std::cout << "Values for "<<s<<": ";
    for (int value : it->second) {
        std::cout << value << " ";
    }
    cout<<endl;
}

