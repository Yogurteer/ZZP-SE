#include "KS2E.h"
#include "utils.h"
#include "inter.h"
#include "DBOGaccess.hpp"

using namespace std;

int main() {

    cout<<"---|KS2E Protocol Local Test Processing|---"<<endl;

    auto start_time = getCurrentTime();

    // DB dir
    // char update_dir1[100] = "result/plain2.txt";

    DBOGaccess db1;
    PGresult* res = nullptr;

    db1.clearTable("TRUNCATE encrypted_file_table;");
    db1.clearTable("TRUNCATE encrypted_index_table;");
    db1.clearTable("TRUNCATE encrypted_index_table_user;");

    // Phrase1.Setup
    cout<<"---|Phrase1:Setup|---"<<endl;

    KS2EUser u;
    KS2EOwner o;
    KS2EServer s;

    u.setup();
    o.setup();
    s.setup();

    map<vector<unsigned char>, vector<vector<unsigned char>>> result;

    // Phrase2.Update1
    cout<<"---|Phrase2:Update1|---"<<endl;

    // read plaintext data from opengauss by user
    std::vector<std::vector<uint8_t>> empty_params; // 空参数
    vector<vector<vector<uint8_t>>> r1 = db1.readData(res, empty_params, "SELECT * FROM cigar;", 0, 0);

    // update enc_index to DB following protocol
    vector<unsigned char> op = {'0'}; // op为0表示添加操作
    vector<vector<vector<uint8_t>>> Ulist=r1;
    for(auto& row : Ulist) {
        for(int i=0;i<16;++i){
            // 明文表中,前16个属性记为关键字,第17个属性表示文件唯一id
            pair w_id = make_pair(row[i], row[16]);
            fullupdate(o, s, w_id, op, db1);
        }
    }

    // update enc_files to DB directly
    for(const auto& row : r1) {
        // 假设每行的第一个元素是文件ID，第二个元素是文件内容
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

    // Phrase.Search1-owner-id 
    cout<<"---|Phrase:Search1-owner-id|---"<<endl;

    vector<vector<unsigned char>> owner_IDq = o.gen_set_query(o.st.ID, 1);

    cout<< "owner Search id: ";printVector(owner_IDq[0]);

    batchsearch_id("owner", o, s, owner_IDq, result, db1);

    o.search_result = result;

    string s1_owner_id_dir = "result/search_owner_id_output.txt";
    search_output_vk(o.search_result, s1_owner_id_dir, db1);
    // search_output(o.search_result, s1_owner_id_dir);

    // Phrase.Search2-owner-w
    cout<<"---|Phrase:Search2-owner-2|---"<<endl;

    vector<vector<unsigned char>> owner_Wq = o.gen_set_query(o.st.W, 1);

    string s_w0(owner_Wq[0].begin(), owner_Wq[0].end());

    cout<< "owner Search w: "<<s_w0<<endl;

    batchsearch_w("owner", o, s, owner_Wq, result, db1);

    o.search_result = result;

    string s2_owner_w_dir = "result/search_owner_w_output.txt";

    // 用户根据按w查询得到的id从加密数据表中搜索得到密文,本地解密
    search_output_kv(o.search_result, s2_owner_w_dir, db1);
    // search_output(o.search_result, s2_owner_w_dir);

    // Phrase3.Sharetoken
    cout<<"---|Phrase3:Sharetoken|---"<<endl;

    vector<unsigned char> share_id = o.gen_set_query(o.st.ID, 1)[0];

    cout<< "Share id: ";printVector(share_id);
 
    vector<vector<unsigned char>> Pid;

    o.sharetoken_id(share_id, Pid);// assume u get Pid

    vector<unsigned char> share_w = o.gen_set_query(o.st.W, 1)[0];

    string s_share_w(share_w.begin(), share_w.end());

    cout<< "Share w: "<<s_share_w<<endl;

    vector<vector<unsigned char>> Pw;

    o.sharetoken_w(share_w, Pw);// assume u get Pw

    // Phrase4.Share
    cout<<"---|Phrase4:Share|---"<<endl;

    fullshare_id(u, s, Pid, db1);

    fullshare_w(u, s, Pw, db1);

    // Phrase5.Search1-w    
    cout<<"---|Phrase5:Search1-w|---"<<endl;

    vector<vector<unsigned char>> Wq = u.gen_set_query(u.st.W, 1);

    string s_w_u(Wq[0].begin(), Wq[0].end());

    cout<< "user Search w: "<<s_w_u<<endl;
    
    batchsearch_w("user", u, s, Wq, result, db1);

    u.search_result = result;

    string s1_w_dir = "result/search_user_w_output.txt";
    // search_output(u.search_result, s1_w_dir);

    // 用户根据按w查询得到的id从加密数据表中搜索得到密文,本地解密
    search_output_kv(u.search_result, s1_w_dir, db1);

    // Phrase6.Search2-id    
    cout<<"---|Phrase6:Search2-id|---"<<endl;
    vector<vector<unsigned char>> IDq = u.gen_set_query(u.st.ID, 1);

    cout<< "user Search id: ";printVector(IDq[0]);

    batchsearch_id("user", u, s, IDq, result, db1);

    u.search_result = result;

    string s2_id_dir = "result/search_user_id_output.txt";
    // search_output(u.search_result, s2_id_dir);
    search_output_vk(u.search_result, s2_id_dir, db1);

    cout<<"---|KS2E Protocol Test Finished|---"<<endl;

    auto end_time = getCurrentTime();
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Total Execution time: " << elapsed_time.count() << " s" << std::endl;

    // 清理
    if (res != nullptr) {
        PQclear(res);
    }

    return 0;
}