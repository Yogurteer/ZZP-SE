#include "KS2E.h"
#include "utils.h"
#include "inter.h"
#include "DBOGaccess.hpp"  // 数据库操作类
#include "memory_utils.h"  // 自定义的内存统计头文件

using namespace std;

const string Protocolexec_record_dir = "result/record12000-1.txt";

DBOGaccess db1("dbname=zzpdb port=5432 host=222.20.126.206 user=admin password=hust@123");
PGresult* res = nullptr;

KS2EClient c;
KS2EUser u;
KS2EOwner o;
KS2EServer s;

map<vector<unsigned char>, vector<vector<unsigned char>>> result;
vector<vector<unsigned char>> Pid; // user按id分享的中间参数
vector<vector<unsigned char>> Pw; // user按w分享的中间参数

void clearAllTables() {
    db1.clearTable("TRUNCATE encrypted_file_table;");
    db1.clearTable("TRUNCATE encrypted_index_table;");
    db1.clearTable("TRUNCATE encrypted_index_table_user;");
}
void setup(){
    u.setup();
    o.setup();
    s.setup();
}
// update方法访问明文数据库，在此设置明文表名
void update(){
    // read plaintext data from opengauss by user
    PGresult* res1 = nullptr;
    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    vector<vector<vector<uint8_t>>> r1 = db1.readData(res1, empty_params, "SELECT * FROM cigar120;", 0, 0);
    PQclear(res1);
    // update enc_index to DB following protocol
    vector<unsigned char> op = {'0'}; // op为0表示添加操作
    vector<vector<vector<uint8_t>>> Ulist=r1;
    int update_count = 0;
    for(auto& row : Ulist) {
        for(int i=0;i<16;++i){
            // 明文表中,前16个属性记为关键字,第17个属性表示文件唯一id
            pair w_id = make_pair(row[i], row[16]);
            fullupdate(o, s, w_id, op, db1);
        }
        update_count++;
        if(update_count % 100 == 0) {
            cout << "Update count: " << update_count << endl;
        }
    }
    // update enc_files to DB directly
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
void search_owner_id(){
    vector<vector<unsigned char>> owner_IDq = o.gen_set_query(o.st.ID, 1);

    cout<< "owner Search id: ";printVector(owner_IDq[0]);

    batchsearch_id("owner", o, s, owner_IDq, result, db1);

    o.search_result = result;

    string s1_owner_id_dir = "result/search_owner_id_output.txt";

    // 用户根据按id查询得到的w
    search_output_vk(o.search_result, s1_owner_id_dir, db1);
}
void search_owner_w(){
    vector<vector<unsigned char>> owner_Wq = o.gen_set_query(o.st.W, 1);

    cout<< "owner Search w: ";printVector(owner_Wq[0]);

    batchsearch_w("owner", o, s, owner_Wq, result, db1);

    o.search_result = result;

    string s2_owner_w_dir = "result/search_owner_w_output.txt";

    // 用户根据按w查询得到的id从加密数据表中搜索得到密文,本地解密
    search_output_kv(o.search_result, s2_owner_w_dir, db1);
}
void sharetoken(){
    vector<unsigned char> share_id = o.gen_set_query(o.st.ID, 1)[0];

    cout<< "Share id: ";printVector(share_id);
 
    o.sharetoken_id(share_id, Pid);// assume u get Pid

    vector<unsigned char> share_w = o.gen_set_query(o.st.W, 1)[0];

    string s_share_w(share_w.begin(), share_w.end());

    cout<< "Share w: "<<s_share_w<<endl;

    o.sharetoken_w(share_w, Pw);// assume u get Pw
}
void share(){
    fullshare_id(u, s, Pid, db1);
    fullshare_w(u, s, Pw, db1);
}
void search_user_w(){
    vector<vector<unsigned char>> Wq = u.gen_set_query(u.st.W, 1);

    string s_w_u(Wq[0].begin(), Wq[0].end());

    cout<< "user Search w: "<<s_w_u<<endl;
    
    batchsearch_w("user", u, s, Wq, result, db1);

    u.search_result = result;

    string s1_w_dir = "result/search_user_w_output.txt";

    // 用户根据按w查询得到的id从加密数据表中搜索得到密文,本地解密
    search_output_kv(u.search_result, s1_w_dir, db1);
}
void search_user_id(){
    vector<vector<unsigned char>> IDq = u.gen_set_query(u.st.ID, 1);

    cout<< "user Search id: ";printVector(IDq[0]);

    batchsearch_id("user", u, s, IDq, result, db1);

    u.search_result = result;

    string s2_id_dir = "result/search_user_id_output.txt";

    search_output_vk(u.search_result, s2_id_dir, db1);
}

int main() {

    ofstream outFile(Protocolexec_record_dir, ios::out);
    if (!outFile) {
        cerr << "Failed to create output file." << endl;
        return 1;
    }

    cout << "---|KS2E Protocol Local Test Processing|---" << endl;

    auto total_start_time = getCurrentTime();
    clearAllTables();

    // Phrase1.Setup
    cout << "---|Setup|---" << endl;
    auto setup_start = getCurrentTime();
    setup();
    auto setup_end = getCurrentTime();
    double setup_time = getDurationSeconds(setup_start, setup_end);
    cout << "Setup Execution time: " << setup_time << " s" << endl;
    outFile << "Setup Execution time: " << setup_time << " s" << endl;

    // Phrase2.Update
    cout << "---|Update|---" << endl;
    auto update_start = getCurrentTime();
    update();
    auto update_end = getCurrentTime();
    double update_time = getDurationSeconds(update_start, update_end);
    cout << "Update Execution time: " << update_time << " s" << endl;
    outFile << "Update Execution time: " << update_time << " s" << endl;

    // Phrase3.Search-owner-id 
    cout << "---|Search-owner-id|---" << endl;
    auto search_owner_id_start = getCurrentTime();
    search_owner_id();
    auto search_owner_id_end = getCurrentTime();
    double search_owner_id_time = getDurationSeconds(search_owner_id_start, search_owner_id_end);
    cout << "Search-owner-id Execution time: " << search_owner_id_time << " s" << endl;
    outFile << "Search-owner-id Execution time: " << search_owner_id_time << " s" << endl;

    // Phrase4.Search-owner-w
    cout << "---|Search-owner-2|---" << endl;
    auto search_owner_w_start = getCurrentTime();
    search_owner_w();
    auto search_owner_w_end = getCurrentTime();
    double search_owner_w_time = getDurationSeconds(search_owner_w_start, search_owner_w_end);
    cout << "Search-owner-w Execution time: " << search_owner_w_time << " s" << endl;
    outFile << "Search-owner-w Execution time: " << search_owner_w_time << " s" << endl;

    // Phrase5.Sharetoken
    cout << "---|Sharetoken|---" << endl;
    auto sharetoken_start = getCurrentTime();
    sharetoken();
    auto sharetoken_end = getCurrentTime();
    double sharetoken_time = getDurationSeconds(sharetoken_start, sharetoken_end);
    cout << "Sharetoken Execution time: " << sharetoken_time << " s" << endl;
    outFile << "Sharetoken Execution time: " << sharetoken_time << " s" << endl;

    // Phrase6.Share
    cout << "---|Share|---" << endl;
    auto share_start = getCurrentTime();
    share();
    auto share_end = getCurrentTime();
    double share_time = getDurationSeconds(share_start, share_end);
    cout << "Share Execution time: " << share_time << " s" << endl;
    outFile << "Share Execution time: " << share_time << " s" << endl;

    // Phrase7.Search-user-w    
    cout << "---|Search-user-w|---" << endl;
    auto search_user_w_start = getCurrentTime();
    search_user_w();
    auto search_user_w_end = getCurrentTime();
    double search_user_w_time = getDurationSeconds(search_user_w_start, search_user_w_end);
    cout << "Search-user-w Execution time: " << search_user_w_time << " s" << endl;
    outFile << "Search-user-w Execution time: " << search_user_w_time << " s" << endl;

    // Phrase8.Search-user-id    
    cout << "---|Search-user-id|---" << endl;
    auto search_user_id_start = getCurrentTime();
    search_user_id();
    auto search_user_id_end = getCurrentTime();
    double search_user_id_time = getDurationSeconds(search_user_id_start, search_user_id_end);
    cout << "Search-user-id Execution time: " << search_user_id_time << " s" << endl;
    outFile << "Search-user-id Execution time: " << search_user_id_time << " s" << endl;

    // Protocol Test Finished
    cout << "---|KS2E Protocol Test Finished|---" << endl;

    // Total execution time
    auto total_end_time = getCurrentTime();
    double total_time = getDurationSeconds(total_start_time, total_end_time);
    cout << "Total Execution time: " << total_time << " s" << endl;
    outFile << endl << "Total Execution time: " << total_time << " s" << endl;

    // Peak memory usage (假设你有实现)
    size_t peak_memory = get_peak_memory_usage(); // 确保你已定义该函数
    cout << "Peak memory usage: " << peak_memory << " KB" << endl;
    outFile << "Peak memory usage: " << peak_memory << " KB" << endl;

    outFile.close();
    if (res != nullptr) {
        PQclear(res);
    }

    return 0;
}