#include "utils.h"
#include "../include/LPSI.h"

// 封装打印 vector<vector<unsigned char>> 的函数
void printYToFile(const vector<vector<unsigned char>>& data, const string& dir, const string& title) {
    ofstream outFile(dir); // 以写模式打开文件，如果文件存在会清空内容
    if (!outFile.is_open()) {
        cerr << "无法打开文件 " << dir << endl;
        return;
    }
    outFile << title << endl;
    for (auto& vec:data){
        outFile << "Key: ";
        for (unsigned char c : vec) {
            outFile << std::hex << static_cast<int>(c) << " ";
        }
        outFile << "\n\n";
    }
    outFile.close(); // 关闭文件
}

// 封装打印 map<vector<unsigned char>, vector<vector<unsigned char>>> 的函数
void printMapToFile(const map<vector<unsigned char>, vector<vector<unsigned char>>>& data, const string& dir, const string& title) {
    ofstream outFile(dir); // 以写模式打开文件，如果文件存在会清空内容
    if (!outFile.is_open()) {
        cerr << "无法打开文件 " << dir << endl;
        return;
    }
    outFile << title << endl;
    for (const auto& pair : data) {
        outFile << "Key: \n";
        for (unsigned char c : pair.first) {
            outFile << std::hex << static_cast<int>(c) << " ";
        }
        outFile << "\nValue:\n";
        for (const auto& vec : pair.second) {
            for (unsigned char c : vec) {
                outFile << std::hex << static_cast<int>(c) << " ";
            }
            outFile << "\n";
        }
        outFile << endl;
    }
    outFile.close(); // 关闭文件
}

// 封装函数打印三维vector并将结果写入文件
void write_ciphertexts_to_file(const std::vector<std::vector<std::vector<unsigned char>>>& ciphertexts, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::out);  // 打开文件进行写入
    if (!outFile) {
        std::cerr << "无法打开文件进行写入!" << std::endl;
        return;
    }

    for (size_t i = 0; i < ciphertexts.size(); ++i) {
        outFile << "ciphertexts[" << i << "]:" << std::endl;
        for (size_t j = 0; j < ciphertexts[i].size(); ++j) {
            outFile << "  ciphertexts[" << i << "][" << j << "]:" << std::endl;
            for (size_t k = 0; k < ciphertexts[i][j].size(); ++k) {
                outFile << "    ciphertexts[" << i << "][" << j << "][" << k << "] = "
                        << std::hex << std::uppercase << std::setw(2) << std::setfill('0')  // 设置十六进制格式
                        << static_cast<int>(ciphertexts[i][j][k]) << std::dec << std::endl;  // 转换为十六进制
            }
        }
    }

    outFile.close();  // 关闭文件
    std::cout << "数据已写入文件: " << filename << std::endl;
}

// 展开tokenlist
void flatten_tokenlist(const std::vector<std::vector<std::vector<unsigned char>>>& all_tokenlist,
                       std::vector<std::vector<unsigned char>>& flattened_tokenlist) {
    // 清空 flattened_tokenlist 以避免以前的内容影响
    flattened_tokenlist.clear();

    // 遍历 all_tokenlist 中的每一行
    for (const auto& row : all_tokenlist) {
        // 遍历每一行中的子vector
        for (const auto& token : row) {
            // 将每个 token 加入 flattened_tokenlist
            flattened_tokenlist.push_back(token);
        }
    }
}

// parse tokens
void parse_tokens(const std::vector<std::vector<std::vector<unsigned char>>>& tokens, 
                  std::vector<std::vector<std::vector<unsigned char>>>& all_tokenlist, 
                  std::vector<std::vector<std::vector<unsigned char>>>& all_symlist) {
    // 清空 all_tokenlist 和 all_symlist 以避免以前的内容影响
    all_tokenlist.clear();
    all_symlist.clear();

    // 遍历 tokens 中的每一行
    for (const auto& row : tokens) {
        // 创建两个新的 vector 来存储当前行的 token 和 sym
        std::vector<std::vector<unsigned char>> current_tokens;
        std::vector<std::vector<unsigned char>> current_syms;

        // 遍历每行中的元素
        for (size_t i = 0; i < row.size(); i += 2) {
            // 偶数索引处是 token，奇数索引处是 sym
            current_tokens.push_back(row[i]);     // token
            current_syms.push_back(row[i + 1]);   // sym
        }

        // 将当前行的 token 和 sym 添加到结果中
        all_tokenlist.push_back(current_tokens);
        all_symlist.push_back(current_syms);
    }
}

// 转换查询键集合为字节向量
void convert_query_set(const std::vector<std::string>& set_query, 
                       std::vector<std::vector<unsigned char>>& vc_set_query) {
    // 清空 vc_set_query 以避免以前的内容影响
    vc_set_query.clear();

    // 遍历 set_query 中的每个 string
    for (const std::string& query : set_query) {
        // 将每个 string 转换为 vector<unsigned char>
        std::vector<unsigned char> byte_vector(query.begin(), query.end());

        // 将转换后的 vector<unsigned char> 添加到 vc_set_query 中
        vc_set_query.push_back(byte_vector);
    }
}

// 转换SToken的键和值为字节向量
void convertmap_string2vc(map<string,vector<vector<unsigned char>>>& SToken,
                          map<vector<unsigned char>, vector<vector<unsigned char>>>& vc_SToken) {
    // 清空 vc_SToken 以避免以前的内容影响
    vc_SToken.clear();

    // 遍历 SToken 中的每个键值对
    for (const auto& pair : SToken) {
        // 获取键（string 类型）
        std::string key = pair.first;
        // 将 string 转换为 vector<unsigned char>
        std::vector<unsigned char> newKey(key.begin(), key.end());

        // 将转换后的键值对插入到 vc_SToken 中
        vc_SToken[newKey] = pair.second;
    }
}

// 解析R，从字符数组转换为unit和char
std::pair<unsigned int, char> parse_R(const std::vector<unsigned char>& pt) {
    if (pt.size() != 5) {
        std::cerr << "错误：字节向量的长度不为5!" << std::endl;
        return std::make_pair(0, '\0');  // 返回一个默认的pair (0, '\0') 表示错误
    }

    unsigned int id = 0;
    char op = 0;
    
    // 将前 4 个字节转换为 unsigned int (大端字节序)
    id |= (static_cast<unsigned int>(pt[0])) << 24;
    id |= (static_cast<unsigned int>(pt[1])) << 16;
    id |= (static_cast<unsigned int>(pt[2])) << 8;
    id |= (static_cast<unsigned int>(pt[3])) << 0;
    
    // 将第 5 个字节转换为 char
    op = static_cast<char>(pt[4]);

    // 返回解析后的结果
    return std::make_pair(id, op);
}

// 转换字符数组为字符串string
void convert_Tokenlist_s(const vector<unsigned char*>& all_tokenlist, vector<string>& s_all_Tokenlist, size_t length) {
    // 遍历 all_tokenlist 中的每个 unsigned char*，并转换为 string
    for (const auto& ptr : all_tokenlist) {
        // 使用 unsigned char* 和长度创建 string
        s_all_Tokenlist.push_back(string(reinterpret_cast<char*>(ptr), length));
    }
}

// 转换字节向量为字节数组
void convert_uc_vc(const vector<vector<vector<unsigned char>>>& multi_LPSI_results, vector<vector<unsigned char*>>& tokens) {
    // 遍历 multi_LPSI_results 的外层 vector
    for (const auto& outer_vec : multi_LPSI_results) {
        vector<unsigned char*> token_inner_vec;
        
        // 遍历每个 inner vector
        for (const auto& inner_vec : outer_vec) {
            // 使用 vector 的 data() 获取 unsigned char* 并将其加入到 token_inner_vec
            const unsigned char* ptr = inner_vec.data();  // 直接获取指向内存的指针
            
            // 将指针加入 token_inner_vec
            token_inner_vec.push_back(const_cast<unsigned char*>(ptr));
        }
        
        // 将 token_inner_vec 加入最终结果 tokens
        tokens.push_back(token_inner_vec);
    }
}

// 辅助函数：将 unsigned char* 转换为 string
string uchar_ptr_to_string(unsigned char* ptr) {
    // 假设unsigned char* 是一个以 '\0' 结尾的 C 字符串
    return string(reinterpret_cast<char*>(ptr));
}

// 转换函数
void convert_EDB_uc(map<unsigned char *, unsigned char *> &EDB, map<string, vector<unsigned char*>> &uc_EDB) {
    for (auto &pair : EDB) {
        // 将 unsigned char* 转换为 string
        string key = uchar_ptr_to_string(pair.first);
        // 将 unsigned char* 转换为 vector<unsigned char*>
        vector<unsigned char*> value = {pair.second};

        // 将转换后的键值对插入到 uc_EDB 中
        uc_EDB[key] = value;
    }
}

// 拼接操作：将 unsigned int id 和 char op 拼接成 unsigned char 数组 id_op
std::vector<unsigned char> concat_id_op(unsigned int id, char op) {
    // 创建一个字节向量，大小为 5 字节
    std::vector<unsigned char> id_op(5);

    // 将 id 的 4 个字节放入 id_op
    for (int i = 0; i < 4; i++) {
        id_op[i] = (id >> (24 - i * 8)) & 0xFF;  // 逐字节取 id
    }

    // 将 op 放入 id_op 的最后一个字节
    id_op[4] = (unsigned char)op;

    // 返回 id_op
    return id_op;
}

// 写入LDSE user获取结果传入 query_set 和 R，将它们写入到指定的文件中,遇到op不为‘0’或‘1’的明文结束写入
void write_LDSE_R_multi(const std::vector<std::string>& query_w, vector<pair<unsigned int, char>> R, const std::string& filename, int max_w_r) {
    // 创建并打开文件
    std::ofstream outFile(filename);

    if (!outFile) {
        std::cerr << "文件打开失败: " << filename << std::endl;
        return;
    }

    // 写入 query_w 中的所有字符串到文件，每个字符串占一行
    int c_cnt = 0;
    for(u_int i=0;i<query_w.size();i++){
        outFile << "query w: " << query_w[i] << endl;
        for(int j=0;j<max_w_r;j++){
            if (R[c_cnt].second == '0' || R[c_cnt].second == '1') {
                outFile << "pt: " << R[c_cnt].first << " " << R[c_cnt].second << endl;
                c_cnt++;
            }
        }
    }

    outFile << std::endl;  // 写入换行符结束 R 的内容

    // 关闭文件
    outFile.close();

    std::cout << "查询结果写入文件: " << filename << std::endl;
}

// 写入LDSE user获取结果传入 query_w 和 R，将它们写入到指定的文件中
void write_LDSE_R_single(const std::string& query_w, const std::vector<unsigned int>& R, const std::string& filename) {
    // 创建并打开文件
    std::ofstream outFile(filename);

    if (!outFile) {
        std::cerr << "文件打开失败: " << filename << std::endl;
        return;
    }

    // 写入 query_w 到文件的第一行
    outFile << query_w << std::endl;

    // 写入 R 中的所有元素到文件的第二行，以空格间隔
    for (size_t i = 0; i < R.size(); ++i) {
        outFile << R[i];
        if (i != R.size() - 1) {
            outFile << " ";  // 在元素之间添加空格
        }
    }

    outFile << std::endl;  // 写入一个换行符结束

    // 关闭文件
    outFile.close();

    std::cout << "查询结果写入文件: " << filename << std::endl;
}

// 随机从W中生成一个w
string getRandomElement(const set<string>& W) {
    // 使用随机数生成器
    random_device rd;  // 随机设备，用于生成种子
    mt19937 gen(rd()); // 生成随机数引擎
    uniform_int_distribution<> dis(0, W.size() - 1); // 随机生成0到W.size()-1的整数

    // 获取一个随机索引
    int random_index = dis(gen);

    // 使用 std::next 获取随机位置的元素
    auto it = next(W.begin(), random_index);  // 通过next从W.begin()向前移动random_index步
    return *it;  // 解引用迭代器，返回对应的元素
}

// 4位字节向量->unsigned int
unsigned int vectorToUInt(const std::vector<unsigned char>& vec) {
    // 假设 vec 中有至少 4 个字节
    if (vec.size() < 4) {
        throw std::invalid_argument("The vector must contain at least 4 bytes.");
    }

    // 通过小端方式将 4 个字节拼接为 unsigned int 类型
    unsigned int result = 0;
    result |= vec[0];          // 低字节
    result |= (vec[1] << 8);   // 第二字节
    result |= (vec[2] << 16);  // 第三字节
    result |= (vec[3] << 24);  // 高字节

    return result;
}

// 随机生成32bit二进制字符串表示key
void generate_random_key(char *key, size_t len) {
    for (size_t i = 0; i < len; i++) {
        key[i] = rand() % 256;  // 生成 0 到 255 之间的随机数，并存储为二进制数据
    }
}

// 将 EDB 转换为 enc_keywordmap
void convertEDBToEncKeywordMap(
    const std::map<std::string, std::vector<std::vector<unsigned char>>>& EDB,
    std::map<std::string, std::vector<std::string>>& enc_keywordmap
) {
    for (const auto& entry : EDB) {
        const std::string& key = entry.first;
        const std::vector<std::vector<unsigned char>>& valueVec = entry.second;

        // 创建一个 vector<string> 来存储转换后的值
        std::vector<std::string> stringVec;
        for (const auto& vec : valueVec) {
            // 将每个 vector<unsigned char> 转换为 string
            stringVec.push_back(vectorToString(vec));
        }

        // 将转换后的结果赋值给 enc_keywordmap
        enc_keywordmap[key] = stringVec;
    }
};

// 将 vector<unsigned char> 转换为 string
std::string vectorToString(const std::vector<unsigned char>& vec) {
    return std::string(vec.begin(), vec.end());
}

// 将 string 转换为 vector<unsigned char>
std::vector<unsigned char> stringToVector(const std::string& str) {
    return std::vector<unsigned char>(str.begin(), str.end());
}

// 函数：生成一个256位的随机数并存储在mpz_t类型中
void generate_256_bit_key(mpz_t key) {
    // 初始化随机数生成器的状态
    gmp_randstate_t state;
    gmp_randinit_default(state);
    
    // 使用时间种子初始化随机数生成器
    unsigned long seed = static_cast<unsigned long>(time(NULL)) ^ (static_cast<unsigned long>(rand()) << 16);
    gmp_randseed_ui(state, seed);

    // 生成 256 位的随机数
    mpz_urandomb(key, state, 256);

    // 清理随机数生成器状态
    gmp_randclear(state);
}

// 读取文件并保存到keywordmap
void read_keywords(char* dir, map<string, vector<unsigned int>>& keywordMap) {
    ifstream file(dir);
    if (!file.is_open()) {
        cerr << "无法打开文件" << endl;
        return;
    }

    string keyword;
    while (getline(file, keyword)) {
        string indicesLine;
        getline(file, indicesLine);
        vector<unsigned int> indices;
        size_t pos = 0;

        // 解析索引集合
        while ((pos = indicesLine.find(' ')) != string::npos) {
            indices.push_back(stoi(indicesLine.substr(0, pos)));
            indicesLine.erase(0, pos + 1);
        }
        if (!indicesLine.empty()) {
            indices.push_back(stoi(indicesLine)); // 插入最后一个索引
        }

        // 将关键字和索引插入到map中
        keywordMap[keyword] = indices;
    }
    file.close();
}

// 函数：从文件中读取大整数p
void read_p_FromFile(const char* filename, mpz_t p) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }

    string line;
    // 读取文件的第一行（应为 "p"）
    getline(infile, line);
    
    // 读取第二行（大整数的二进制字符串）
    if (getline(infile, line)) {
        // 使用 mpz_set_str 将读取的二进制字符串转换为大整数
        mpz_set_str(p, line.c_str(), 2);  // 基数 2 表示二进制
    } else {
        cerr << "未能读取大整数。" << endl;
    }

    infile.close();
}

// 将mpz_t input截断1024bit赋值给output
void truncate_to_1024bit(mpz_t output, const mpz_t input) {
    // 定义掩码，将值截断至 1024 位 (0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    // ... 共 1024 位)
    mpz_t mask;
    mpz_init(mask);

    // 将掩码设置为 2^1024 - 1，这样就会得到 1024 位的全 1
    mpz_ui_pow_ui(mask, 2, 1024); // 计算 2^1024
    mpz_sub_ui(mask, mask, 1);    // 减去 1 得到 1024 位全 1 掩码

    // 对输入进行按位与操作，保留最低 1024 位
    mpz_and(output, input, mask);

    // 清理掩码
    mpz_clear(mask);
}

// 将 std::vector<string> s_y_set 中的每个元素求 SHA256 并截断 256bit 再转化为 mpz_t 类型，结果保存为 std::vector<mpz_t> y_set
void s_y_set_to_h_y_set(std::vector<mpz_t>& y_set, std::vector<std::vector<unsigned char>>& s_y_set) {
    if (s_y_set.size() != y_set.size()) {
        throw std::runtime_error("Error: s_y_set and y_set must have the same size.");
    }

    for (size_t i = 0; i < s_y_set.size(); ++i) {
        // Step 1: Use the vector<unsigned char> directly
        std::vector<unsigned char>& data = s_y_set[i];

        // Step 2: Compute SHA256 of the vector<unsigned char>
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.data(), data.size()); // Use data.data() to get the pointer to the vector's data
        SHA256_Final(hash, &sha256);

        // Step 3: Truncate the hash to 32 bits (4 bytes)
        unsigned int hx = 0;
        for (int j = 0; j < 4; ++j) {
            hx = (hx << 8) | hash[j];
        }

        // Step 4: Convert hx to mpz_t and store it in y_set
        mpz_set_ui(y_set[i], hx);
    }
}

// 从 map<string, vector<int>> 中不重复地随机选择 n 个键
std::vector<std::string> getRandomKeysFromMap(const std::map<std::string, std::vector<int>>& keywordMap, size_t n) {
    if (keywordMap.empty()) {
        throw std::runtime_error("Error: keywordMap is empty.");
    }
    if (n > keywordMap.size()) {
        throw std::runtime_error("Error: n is greater than the number of available keys.");
    }

    // 将所有键存入一个向量
    std::vector<std::string> keys;
    for (const auto& pair : keywordMap) {
        keys.push_back(pair.first);
    }

    // 使用随机数生成器进行洗牌
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 随机数生成器
    std::shuffle(keys.begin(), keys.end(), gen);

    // 选择前 n 个键
    std::vector<std::string> selectedKeys(keys.begin(), keys.begin() + n);

    return selectedKeys;
}

// 从b_pairs.txt文件中读取两个大整数赋值给mpz_t b和mpz_t b_inv
void read_b_pairs_from_file(const std::string &filename, mpz_t b, mpz_t b_inv) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    // 读取第一行（pair1），跳过
    std::getline(inFile, line);

    // 读取b的二进制字符串
    std::getline(inFile, line); // 读取 "b"
    std::getline(inFile, line); // 读取对应的二进制字符串
    mpz_set_str(b, line.c_str(), 2); // 将二进制字符串转换为mpz_t类型

    // 读取b_inv的二进制字符串
    std::getline(inFile, line); // 读取 "b_inv"
    std::getline(inFile, line); // 读取对应的二进制字符串
    mpz_set_str(b_inv, line.c_str(), 2); // 将二进制字符串转换为mpz_t类型

    inFile.close();
}

// 子函数：加密 CuckooHashTable 并赋值给 EncCuckooHashTable
void encryptCuckooHashTable(
    const CuckooHashTable &cuckoo, 
    EncCuckooHashTable &enc_cuckoo, 
    const mpz_t p, 
    const mpz_t ksym) {

    mpz_t result;
    mpz_init(result); // 初始化 mpz_t 类型的变量用于存储加密结果

    // 加密 table 中的每个元素
    for (size_t i = 0; i < cuckoo.table.size(); ++i) {
        if (!cuckoo.table[i].empty()) { // 如果当前元素不为空
            // 调用加密函数对 cuckoo.table[i] 加密
            F_k_x(result, cuckoo.table[i], p, ksym);
            // 将加密结果转化为字符串
            char *encrypted_str = mpz_get_str(nullptr, 16, result);
            // 赋值给 enc_cuckoo 的对应位置
            enc_cuckoo.table[i] = std::string(encrypted_str);
            // 释放内存
            free(encrypted_str);
        }
    }

    // 加密 stash 中的每个元素
    for (size_t i = 0; i < cuckoo.stash.size(); ++i) {
        if (!cuckoo.stash[i].empty()) { // 如果当前元素不为空
            // 调用加密函数对 cuckoo.stash[i] 加密
            F_k_x(result, cuckoo.table[i], p, ksym);
            // 将加密结果转化为字符串
            char *encrypted_str = mpz_get_str(nullptr, 16, result);
            // 赋值给 enc_cuckoo 的对应位置
            enc_cuckoo.table[i] = std::string(encrypted_str);
            // 释放内存
            free(encrypted_str);
        }
    }

    mpz_clear(result); // 清除 mpz_t 类型变量以释放内存
}

// 将 mpz_t 转换为 16 进制字符串
std::string mpzToHexString(const mpz_t num) {
    char* hex_str = mpz_get_str(nullptr, 16, num); // 将 mpz_t 转换为 16 进制字符串
    std::string result(hex_str);
    free(hex_str); // 释放动态分配的内存
    return result;
}

// // 将 CuckooHashTable 中的数据保存到文件
// void saveCuckooHashTableToFile(const CuckooHashTable& hashTable) {
//     // 打开文件进行写操作
//     std::ofstream outFile("view_cuckoo.txt");

//     if (!outFile) {
//         std::cerr << "Error: Could not open the file for writing." << std::endl;
//         return;
//     }

//     // 输出哈希表信息到文件
//     outFile << "CuckooHashTable Information:" << std::endl;
//     outFile << "Original Element Count (n): " << hashTable.n << std::endl;
//     outFile << "Hash Function Count (k): " << hashTable.k << std::endl;
//     outFile << "Table Elements:" << std::endl;

//     // 保存哈希表中的元素
//     for (size_t i = 0; i < hashTable.table.size(); ++i) {
//         outFile << "table[" << i << "]: " 
//                 << (hashTable.table[i].empty() ? "empty" : hashTable.table[i]) 
//                 << std::endl;
//     }

//     outFile << "Stash Elements:" << std::endl;

//     // 保存 stash 中的元素
//     if (hashTable.stash.empty()) {
//         outFile << "Stash is empty." << std::endl;
//     } else {
//         for (size_t i = 0; i < hashTable.stash.size(); ++i) {
//             outFile << "stash[" << i << "]: " << hashTable.stash[i] << std::endl;
//         }
//     }

//     // 关闭文件
//     outFile.close();

//     std::cout << "Cuckoo hash table information has been saved to view_cuckoo.txt." << std::endl;
// }

//mpz转string
std::string mpzToString(const mpz_t value) {
    // 使用 GMP 的 mpz_get_str 函数将 mpz_t 转换为 C 字符串。
    // "16" 表示转换为十六进制字符串。
    char* hexStr = mpz_get_str(nullptr, 16, value);

    // 将 C 字符串转换为 std::string
    std::string result(hexStr);

    // 释放由 mpz_get_str 分配的内存
    free(hexStr);

    return result;
}

// string to vector
std::vector<unsigned char> hexstringToVector(const std::string& keyHex) {
    // 将十六进制字符串转换为字节
    std::vector<unsigned char> key;
    for (size_t i = 0; i < keyHex.length(); i += 2) {
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << keyHex.substr(i, 2);
        ss >> byte;
        key.push_back(static_cast<unsigned char>(byte));
    }
    return key;
}
