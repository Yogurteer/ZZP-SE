#include "utils.h"

std::chrono::high_resolution_clock::time_point getCurrentTime() {
    return std::chrono::high_resolution_clock::now();
}

void mergeFiles(const std::string& file1, const std::string& file2, const std::string& outputFile) {
    std::unordered_map<std::string, std::vector<int>> data;

    // 读取第一个文件
    std::ifstream inFile1(file1);
    if (inFile1.is_open()) {
        std::string line;
        std::string key;
        while (std::getline(inFile1, line)) {
            if (std::all_of(line.begin(), line.end(), ::isalpha)) {
                key = line;
            } else {
                std::istringstream iss(line);
                int num;
                while (iss >> num) {
                    data[key].push_back(num);
                }
            }
        }
        inFile1.close();
    }

    // 读取第二个文件
    std::ifstream inFile2(file2);
    if (inFile2.is_open()) {
        std::string line;
        std::string key;
        while (std::getline(inFile2, line)) {
            if (std::all_of(line.begin(), line.end(), ::isalpha)) {
                key = line;
            } else {
                std::istringstream iss(line);
                int num;
                while (iss >> num) {
                    data[key].push_back(num);
                }
            }
        }
        inFile2.close();
    }

    // 写入合并后的文件
    std::ofstream outFile(outputFile);
    if (outFile.is_open()) {
        for (const auto& pair : data) {
            outFile << pair.first << std::endl;
            for (size_t i = 0; i < pair.second.size(); ++i) {
                if (i > 0) {
                    outFile << " ";
                }
                outFile << pair.second[i];
            }
            outFile << std::endl;
        }
        outFile.close();
    }
}

void generate_random_key(char *key, size_t len) {
    for (size_t i = 0; i < len; i++) {
        key[i] = rand() % 256;  // 生成 0 到 255 之间的随机数，并存储为二进制数据
    }
}

std::vector<unsigned char> toByteVector(const char* px) {
    return std::vector<unsigned char>(px, px + strlen(px));  // 直接转换
}

std::vector<unsigned char> uintToBytes(unsigned int id) {
    std::vector<unsigned char> bytes(4);
    bytes[0] = (id >> 24) & 0xFF;
    bytes[1] = (id >> 16) & 0xFF;
    bytes[2] = (id >> 8) & 0xFF;
    bytes[3] = id & 0xFF;
    return bytes;
}

vector<unsigned char> concatenate_vectors(const std::vector<unsigned char>& L_1, const std::vector<unsigned char>& L_2) {
    // 创建一个新的 vector，其大小为 L_1 和 L_2 的总和
    std::vector<unsigned char> L_3;
    
    // 将 L_1 插入到 L_3
    L_3.insert(L_3.end(), L_1.begin(), L_1.end());
    
    // 将 L_2 插入到 L_3
    L_3.insert(L_3.end(), L_2.begin(), L_2.end());

    return L_3;
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

void get_L_1_ks_1(std::vector<unsigned char>& L_1_ks_1,std::vector<unsigned char>& L_1,std::vector<unsigned char>& ks_1, size_t f_len) {
    // 确保L_1_ks_1的长度至少为8
    if (L_1_ks_1.size() >= f_len*2) {
        // 将L_1_ks_1的前4字节赋值给L_1
        L_1.assign(L_1_ks_1.begin(), L_1_ks_1.begin() + f_len);
        
        // 将L_1_ks_1的后4字节赋值给ks_1
        ks_1.assign(L_1_ks_1.begin() + f_len, L_1_ks_1.begin() + f_len*2);
    } else {
        cout<<"L_1_ks_1:";
        for (unsigned char byte : L_1_ks_1) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
        }
        std::cout << std::endl;
        std::cerr << "L_1_ks_1 must have at least " << f_len <<"elements." << std::endl;
    }
}

unsigned int bytesToUint(const std::vector<unsigned char>& bytes) {
    unsigned int id = 0;
    id |= static_cast<unsigned int>(bytes[0]) << 24;
    id |= static_cast<unsigned int>(bytes[1]) << 16;
    id |= static_cast<unsigned int>(bytes[2]) << 8;
    id |= static_cast<unsigned int>(bytes[3]);
    return id;
}