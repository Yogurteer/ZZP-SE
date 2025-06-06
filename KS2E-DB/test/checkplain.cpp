// g++ checkplain.cpp -o check

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

int main() {
    std::ifstream file("../result/plain.txt");
    if (!file.is_open()) {
        std::cerr << "无法打开文件！" << std::endl;
        return 1;
    }

    std::unordered_map<std::string, int> lineCounts;  // key = 行内容，value = 出现次数
    std::string line;

    while (std::getline(file, line)) {
        ++lineCounts[line];  // 每次读取一行就 +1
    }

    file.close();

    int duplicateCount = 0;

    std::cout << "以下是重复的行及其出现次数：\n";
    for (const auto& entry : lineCounts) {
        if (entry.second > 1) {
            std::cout << "行内容: \"" << entry.first << "\" 出现次数: " << entry.second << '\n';
            ++duplicateCount;
        }
    }

    std::cout << "\n🔁 重复的行总数为：" << duplicateCount << " 条" << std::endl;

    return 0;
}
