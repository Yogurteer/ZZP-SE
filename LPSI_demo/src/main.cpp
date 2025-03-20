#include "utils.h"
#include "LPSI.h"
#include "sender.h"
#include "receiver.h"
#include "LPSI.h"

using namespace std;

uint size_query = 2; //单次查询的关键字个数

int main() {
    auto start_time = std::chrono::high_resolution_clock::now();

    // multi-point LPSI protocol-demo

    // sender input: |X| and corresponding Labels
    // keyword和对应label的基本单位类型是vector<unsigned char>
    map<vector<unsigned char>, vector<vector<unsigned char>>> X_Labels = {
        {{0x01, 0x02, 0x03}, {{0x10, 0x20}, {0x30, 0x40}}},
        {{0x04, 0x05}, {{0x50, 0x60, 0x70}, {0x80, 0x90}}},
        {{0x06, 0x07}, {{0x01, 0x02}, {0x03}}}
    };

    string Xdir = "result/X.txt";
    string Ydir = "result/Y.txt";
    string outputdir = "result/output.txt";

    // 打印 X_Labels 到文件
    printMapToFile(X_Labels, Xdir, "X_Labels:");

    // Initial Y
    vector<vector<unsigned char>> Y = {
        {0x01, 0x02, 0x03}, {0x04, 0x05}
    };
    printYToFile(Y, Ydir, "Y:");

    // run multi-point LPSI protocol
    map<vector<unsigned char>, vector<vector<unsigned char>>> multi_LPSI_results;
    multi_LPSI(X_Labels.size(), Y.size(), X_Labels, Y, multi_LPSI_results);

    // 打印 multi_LPSI_results 到文件
    printMapToFile(multi_LPSI_results, outputdir, "multi_LPSI_results:");

    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算并输出协议执行时间
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Multi-point LPSI time: " << elapsed_time.count() << " s" << std::endl;

    return 0;
}
