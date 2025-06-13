#include <iostream>
#include <unistd.h>
#include <sys/resource.h>

// #include <iostream>  // 用于标准输入输出（cerr）
// #include <fstream>   // 用于文件流操作（ifstream）
// #include <sstream>
// #include <string>    // 用于字符串处理（string）
// #include <vector>    // 用于存储索引的动态数组（vector）
// #include <unordered_map> // 用于索引的哈希表（unordered_map）
// #include <map>       // 用于关键字和索引的映射（map）
// #include <set>
// #include <string.h>
// #include <gmp.h>     // 用于大数运算
// #include <chrono> // 包含chrono库用于测量时间
// #include <cmath>
// #include <random>
// #include <algorithm>
// #include <iomanip>

size_t get_peak_memory_usage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // ru_maxrss 是以 KB 为单位的最大 RSS（Resident Set Size）
        return static_cast<size_t>(usage.ru_maxrss);
    }
    return 0;
}

int main() {
    size_t peak = get_peak_memory_usage();
    std::cout << "Minimal Peak Memory: " << peak << " KB\n";
    return 0;
}