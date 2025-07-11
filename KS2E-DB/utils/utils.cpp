#include "utils.h"

std::string vectorToHexStr(const std::vector<uint8_t>& datavec) {
    std::stringstream ss;

    for (const auto& byte : datavec) {
        ss << std::setw(2) << std::setfill('0') << std::hex 
           << static_cast<int>(byte);
    }

    return ss.str();
}

vector<unsigned char> encodeWithDelimiter(const vector<vector<unsigned char>>& input) {
    vector<unsigned char> result;
    for (size_t i = 0; i < input.size(); ++i) {
        result.insert(result.end(), input[i].begin(), input[i].end());
        if (i != input.size() - 1) {
            result.push_back(DELIMITER);  // 添加分隔符
        }
    }
    return result;
}

vector<vector<unsigned char>> decodeWithDelimiter(const vector<unsigned char>& input) {
    vector<vector<unsigned char>> result;
    vector<unsigned char> current;

    for (unsigned char byte : input) {
        if (byte == DELIMITER) {
            result.push_back(current);
            current.clear();
        } else {
            current.push_back(byte);
        }
    }
    if (!current.empty()) {
        result.push_back(current); // 添加最后一部分
    }

    return result;
}

void printVector(const std::vector<uint8_t>& datavec) {
    for (size_t j = 0; j < datavec.size(); ++j) {
        // 打印每个字节，使用十六进制格式
        std::cout << std::setw(2) << std::setfill('0') 
                    << std::hex << (int)datavec[j];
        std::cout << std::dec;  // 设置为十进制输出
    }
    cout<<endl;
}

void view_pt(const std::vector<unsigned char>& v_w) {
    string w(v_w.begin(), v_w.end());
    cout<<w;
}

void view_vc(const std::vector<unsigned char>& vc) {
    for (auto c : vc) {
        std::cout << static_cast<int>(c) << " ";
    }
    std::cout << std::endl;
}

std::vector<unsigned char> uint2vc(unsigned int value) {
    return {
        static_cast<unsigned char>((value >> 24) & 0xFF),
        static_cast<unsigned char>((value >> 16) & 0xFF),
        static_cast<unsigned char>((value >> 8) & 0xFF),
        static_cast<unsigned char>(value & 0xFF)
    };
}

unsigned int vc2uint(const std::vector<unsigned char>& vec) {
    if (vec.size() != 4) {
        throw std::invalid_argument("Vector size must be 4 for vc2uint");
    }

    return (static_cast<unsigned int>(vec[0]) << 24) |
           (static_cast<unsigned int>(vec[1]) << 16) |
           (static_cast<unsigned int>(vec[2]) << 8)  |
           (static_cast<unsigned int>(vec[3]));
}

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

std::vector<unsigned char> toByteVector(const unsigned char* px, size_t len) {
    return std::vector<unsigned char>(px, px + len);
}

std::vector<unsigned char> uintToBytes(unsigned int id) {
    std::vector<unsigned char> bytes(4);
    bytes[0] = (id >> 24) & 0xFF;
    bytes[1] = (id >> 16) & 0xFF;
    bytes[2] = (id >> 8) & 0xFF;
    bytes[3] = id & 0xFF;
    return bytes;
}

void read_keywords(const char* dir, map<string, vector<string>>& keywordMap) {
    ifstream file(dir);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << dir << endl;
        return;
    }

    string keyword;
    while (getline(file, keyword)) {
        string indicesLine;
        
        if (!getline(file, indicesLine)) {
            cerr << "文件格式错误：关键字 " << keyword << " 没有对应的 ID 行" << endl;
            break;
        }

        vector<string> indices;
        istringstream iss(indicesLine);
        string id;
        while (iss >> id) {
            indices.push_back(id);  // 保留为字符串
        }

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

void search_output_vk(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, 
                   const std::string& search_output_dir, DBOGaccess& db1) {
    string file_path = search_output_dir;
    // 打开文件
    std::ofstream outfile(file_path);

    if (!outfile.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + file_path);
    }

    for (const auto& entry : search_result) {
        vector<unsigned char> v_file_id = entry.first;
        auto keywords = entry.second;
        // 写入kv
        outfile << "file ID: " << endl;
        string hexstr = vectorToHexStr(v_file_id);
        outfile << hexstr << std::endl;

        outfile << "keyword: ";
        if(keywords.size()==0){
            outfile << " NULL" << endl;
            continue;
        }
        for (auto v_keyword : keywords) {
            vector<unsigned char> keyword = v_keyword;
            string str_keyword(keyword.begin(), keyword.end());// 还原keyowrd类型为string
            outfile << endl << str_keyword; // 每个 keyword 之间空格
        }
        outfile << std::endl;
    }

    // 关闭文件
    outfile.close();

    cout<<"查询结果已写入"<<search_output_dir<<endl;
}

void search_output_kv(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, 
                   const std::string& search_output_dir, DBOGaccess& db1) {
    PGresult* res;

    string file_path = search_output_dir;
    // 打开文件
    std::ofstream outfile(file_path);

    if (!outfile.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + file_path);
    }

    for (const auto& entry : search_result) {
        vector<unsigned char> v_w = entry.first;
        string w(v_w.begin(), v_w.end());// 还原keyword类型为string
        auto ids = entry.second;
        // 写入kv
        outfile << "keyword: " << endl << w << std::endl;
        outfile << "file content: ";
        if(ids.size()==0){
            outfile << " NULL" << endl;
            continue;
        }
        for (auto v_id : ids) {
            vector<unsigned char> file_id = v_id;
            vector<vector<unsigned char>> params = {file_id};
            
            auto r = db1.readData(res, params, "SELECT encrypted_file FROM encrypted_file_table WHERE file_id = $1", 1, 1);
            
            vector<unsigned char> enc_file = r[0][0]; // 获取加密的文件内容
            vector<unsigned char> file = my_xor(enc_file, file_id); 
            string file_content(file.begin(), file.end());// 还原id类型为string
            outfile << endl << file_content; // 每个 id 之间空格
        }
        outfile << std::endl;
    }

    // 关闭文件
    outfile.close();

    // 清理
    if (res != nullptr) {
        PQclear(res);
    }

    cout<<"查询结果已写入"<<search_output_dir<<endl;
}

void search_output(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, 
                   const std::string& search_output_dir) {
        
    string file_path = search_output_dir;
    // 打开文件
    std::ofstream outfile(file_path);

    if (!outfile.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + file_path);
    }

    for (const auto& entry : search_result) {
        vector<unsigned char> v_w = entry.first;
        string w(v_w.begin(), v_w.end());// 还原keyword类型为string
        auto ids = entry.second;
        // 写入 w 和 id 信息
        outfile << "w: " << endl << w << std::endl;
        outfile << "id:";
        if(ids.size()==0){
            outfile << " NULL" << endl;
            continue;
        }
        for (auto v_id : ids) {
            string id(v_id.begin(), v_id.end());// 还原id类型为string
            outfile << endl << id; // 每个 id 之间空格
        }
        outfile << std::endl;
    }

    // 关闭文件
    outfile.close();

    cout<<"查询结果已写入"<<search_output_dir<<endl;
}