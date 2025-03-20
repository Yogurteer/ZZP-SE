#include "com.h"

// 将Token写入文件,输入二维字节向量
void write_Token(const vector<vector<unsigned char>>& Token, const string& Token_dir) {
    // 打开文件输出流
    ofstream outFile(Token_dir, ios::binary);
    
    // 检查文件是否成功打开
    if (!outFile) {
        cerr << "无法打开文件 " << Token_dir << " 写入数据!" << endl;
        return;
    }

    // 遍历每一行
    for (const auto& row : Token) {
        for (const auto& value : row) {
            // 将每个unsigned char写入文件
            outFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        outFile.put('\n');  // 每一行结束后换行
    }

    // 关闭文件
    outFile.close();
}

// 从文件读取Token,输出二维字节向量
vector<vector<unsigned char>> read_Token(const string& Token_dir) {
    // 打开文件输入流
    ifstream inFile(Token_dir, ios::binary);

    // 检查文件是否成功打开
    if (!inFile) {
        cerr << "无法打开文件 " << Token_dir << " 读取数据!" << endl;
        return {};
    }

    vector<vector<unsigned char>> Token;
    vector<unsigned char> row;
    unsigned char value;
    
    // 逐字节读取文件内容
    while (inFile.read(reinterpret_cast<char*>(&value), sizeof(value))) {
        if (inFile.peek() == '\n' || inFile.peek() == EOF) {
            // 换行表示当前行结束，将当前行添加到Token中
            Token.push_back(row);
            row.clear();  // 清空当前行
        } else {
            row.push_back(value);
        }
    }

    // 处理最后一行（没有换行符的情况）
    if (!row.empty()) {
        Token.push_back(row);
    }

    // 关闭文件
    inFile.close();
    
    return Token;
}