
/*
overalltest for all protocols with update and search
请完善自己的方案所在的测试函数,如test_KS2E(),测试框架参考test_KS2E()
方案核心算法放在include目录,功能函数放在utils目录
方案中,请将client与server构造为不同的类,分开实现各自的方法
在更新,搜索功能的核心算法替换为自己方案中的已实现的方法,
将核心算法函数的命名和参数类型规范为:
(如果下列规范与方案有不可避免的冲突,可适当修改)
XXXX:方案名,如KS2E

1.client update
void XXXXClient::update(vector<unsigned char> w, vector<unsigned char> id, vector<unsigned char> op, vector<vector<unsigned char>>& tokens)
输入:
w: 关键字
id: 文件ID
op: 操作符,1表示插入,0表示删除
输出:
tokens: 中间参数

2.server update
void XXXXServer::update(vector<vector<unsigned char>> Cwid, DBOGaccess& db1)
输入:
tokens: 中间参数
db1: 访问opengauss数据库的指针
输出: 
无

3.client search
void KS2EClient::search(vector<unsigned char> w, vector<vector<unsigned char>>& request)
输入:
w: 关键字
输出:
request: 查询请求

4.server search
void KS2EServer::search(vector<vector<unsigned char>> request, vector<vector<unsigned char>>& response, DBOGaccess& db1)
输入:
request: 查询请求
db1: 访问opengauss数据库的指针
输出:
response: 查询响应

5.client callback
void KS2EClient::callback(vector<vector<unsigned char>> param, vector<vector<unsigned char>> response, vector<vector<unsigned char>>& result)
输入:
param: 解码所需参数
response: 查询响应
输出:
result: 解码结果
*/

// 包含方案所需的头文件
// #include "../include/KS2E.h" // KS2E方案核心功能函数头文件
#include "../utils/DBOGaccess.hpp"  // 自定义的opengauss数据库操作类
#include "../utils/memory_utils.h"  // 自定义的内存统计头文件
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map> 
#include <map>       
#include <set>


using namespace std;

// 公共参数,所有协议通用
const string Protocolexec_record_dir = "result/record120-2.txt";// 记录协议执行结果的文件
int plain_rows=0;// 记录明文数据行数
set<vector<unsigned char>> K_set;// 记录出现过的关键字空间
DBOGaccess db1("dbname=zzpdb port=5432 host=222.20.126.206 user=admin password=hust@123");// 设置opengauss数据库连接信息
string table_name = "cigar120";// 设置明文数据所在table
PGresult* res = nullptr; // 用于存储DB查询结果的指针
const string search_record_path = "result/"; // 存放搜索结果的目录

// clear enc_file_table, encrypted_index_table and other tables related to yout Protocol
void clearAllTables() {
    db1.clearTable("TRUNCATE encrypted_file_table;");
    db1.clearTable("TRUNCATE encrypted_index_table;");
    db1.clearTable("TRUNCATE encrypted_index_table_user;");
}

// update enc_file following key management protocol,default enc_file = xor(file, fileID)
void update_enc_file(){
    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    string query_all_command = "SELECT * FROM " + table_name + ";";
    vector<vector<vector<uint8_t>>> r1 = db1.readData(res, empty_params, query_all_command.c_str(), 0, 0);
    // update enc_files to DB following key management protocol
    for(const auto& row : r1) {
        // encrypted_file_table第一列是文件ID，第二列是文件内容
        vector<unsigned char> fileID=row[16];
        vector<vector<unsigned char>> entry;
        for(int i = 0; i < 16; ++i) {
            entry.push_back(row[i]);
        }
        vector<unsigned char> file = encodeWithDelimiter(entry);
        vector<unsigned char> enc_file = my_xor(file, fileID); // 正常应该是用唯一的文件密钥加密文件
        // client send filedID and enc_file to server
        vector<vector<unsigned char>> datavecs = {fileID, enc_file};
        db1.writeData(datavecs, "INSERT INTO encrypted_file_table (file_id, encrypted_file) VALUES ($1, $2)", 1);
    }
}

void test_xx(){
    string protocol_name = "xxxx";

    // Create output file for protocol execution records
    ofstream outFile(Protocolexec_record_dir, ios::out);
    if (!outFile) {
        cerr << "Failed to create output file." << endl;
        return;
    }

    // Start protocol execution
    cout << "---|"<<protocol_name<<" Test Processing|---" << endl;
    outFile << "---|"<<protocol_name<<" Test|---" << endl;
    auto total_start_time = getCurrentTime(); // record start time

    // Phrase1.Setup
    cout << "---|Setup|---" << endl;
    auto setup_start = getCurrentTime();

    //***根据方案执行setup相关操作***//

    //***分隔符***//

    auto setup_end = getCurrentTime();
    double setup_time = getDurationSeconds(setup_start, setup_end);
    outFile << "Setup Execution time: " << setup_time << " s" << endl;

    // Phrase2.Update
    cout << "---|Update|---" << endl;
    auto update_start = getCurrentTime();

    // read plaintext data from opengauss by user
    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    string query_all_command = "SELECT * FROM " + table_name + ";";
    vector<vector<vector<uint8_t>>> r1 = db1.readData(res, empty_params, query_all_command.c_str(), 0, 0);
    plain_rows = r1.size();

    // update enc_index to DB following protocol
    vector<unsigned char> op = {'0'}; // op为0表示添加操作,1表示删除
    int update_count = 0;
    for(auto& row : r1) {
        for(int i=0;i<16;++i){
            // 明文表中,前16个属性记为关键字,第17个属性表示文件唯一id
            vector<unsigned char> w = row[i];
            K_set.insert(w);
            vector<unsigned char> id = row[16];
            vector<vector<unsigned char>> tokens; // update时client与server中间参数

            //***client端update***//
            
            //***分隔符***//

            //***server端update***//
            
            //***分隔符***//

        }
        update_count++;
        if(update_count % 100 == 0) {// 每更新100条记录输出一次,用于大规模数据集测试
            cout << "Update count: " << update_count << endl;
        }
    }

    auto update_end = getCurrentTime();
    double update_time = getDurationSeconds(update_start, update_end);
    outFile << "Update Execution time: " << update_time << " s" << endl;
    outFile << "Plain rows: " << plain_rows << endl;

    // Phrase3.Search
    cout << "---|Search|---" << endl;
    auto search_owner_w_start = getCurrentTime();
    vector<vector<unsigned char>> select_queries(K_set.begin(), K_set.end());// 生成查询w,设置等于出现过的所有关键字
    size_t query_size = select_queries.size();
    map<vector<unsigned char>, vector<vector<unsigned char>>> results; // 存储多个关键字下的搜索结果
    string client_type = "owner"; // 客户端类型,KS2E有owner和user两种客户端类型
    results.clear();
    for(auto w:select_queries) {

        //***client端search***//

        //***分隔符***//

        //***server端search***//

        //***分隔符***//

        //***client端callback***//

        //***分隔符***//

        results[w] = result;
    }

    // 用户根据按w查询得到的id从加密数据表中搜索得到密文,本地解密,写入文件search_record_dir
    string search_record_dir = search_record_path + protocol_name + "_search_w.txt";
    search_output_kv(results, search_record_dir, db1); // results={(keyword1,fileid1),(keyword2,fileid2)...}
    auto search_owner_w_end = getCurrentTime();
    double search_owner_w_time = getDurationSeconds(search_owner_w_start, search_owner_w_end);
    outFile << "Search size: " << query_size << endl;
    outFile << "Search Execution time: " << search_owner_w_time << " s" << endl;

    // End protocol execution
    cout << "---|Protocol Test Finished|---" << endl;
    auto total_end_time = getCurrentTime();
    double total_time = getDurationSeconds(total_start_time, total_end_time);
    cout << "Total Execution time: " << total_time << " s" << endl;
    outFile << endl << "Total Execution time: " << total_time << " s" << endl;

    // Peak memory usage
    size_t peak_memory = get_peak_memory_usage(); // 确保你已定义该函数
    cout << "Peak memory usage: " << peak_memory << " KB" << endl;
    outFile << "Peak memory usage: " << peak_memory << " KB" << endl;
}

void test_Mitra(){
    // Mitrs;
}

void test_ODXT(){
    // ODXT;
}

void test_Bestie(){
    // Bestie;
}

void test_Fides(){
    // Fides;
}

void test_KS2E(){
    // KS2E;
}

int main(){
    clearAllTables(); // 清空所有密文表

    update_enc_file(); // 更新加密文件表

    test_xx();
    // test_KS2E();
    // test_Mitra();
    // test_ODXT();
    // test_Bestie();
    // test_Fides();

    if (res != nullptr) {
        PQclear(res);
    }
}