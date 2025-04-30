#include "FPKS2E.h"
#include "utils.h"

using namespace std;

void batchupdate(FPKS2EClient& c, FPKS2EServer& s, char *update_dir, vector<unsigned char> op, int version)
{
    std::cout << "Updating FPKS2E data..." << std::endl;
    // 从文件读入DB
    map<string, vector<unsigned int>> Ulist;
    Ulist.clear(); // 清空Ulist
    cout << "Update Dataset source dir: " << update_dir << endl;
    read_keywords(update_dir, Ulist);

    // 遍历Ulist中的每个关键字
    for (const auto& entry : Ulist) {
        string w = entry.first;         // 获取关键字
        vector<unsigned char> v_w(w.begin(), w.end());
        vector<unsigned int> ids = entry.second;       // 获取对应的id集合
        for(auto &id:ids){
            auto v_id = uint2vc(id);
            pair w_id = make_pair(v_w, v_id);

            vector<vector<unsigned char>> Cwid = {};

            if(version==1){
                c.update_1(w_id, op, Cwid);
            }
            else if(version==2){
                c.update_2(w_id, op, Cwid);
            }
            // server receive and update
            s.re_update(Cwid);
        }
    }
}

int main() {

    cout<<"---|Simple-KS2E Protocol Processing|---"<<endl;

    auto start_time = getCurrentTime();

    // DB dir
    char update_dir1[100] = "dataset/update_data_mini/add/Ulist1.txt"; // 初始化数据目录,即Ulist1
    char update_dir2[100] = "dataset/update_data_mini/add/Ulist2.txt"; // 第一次更新-add

    string check_dir = "dataset/view/Ulist1p2.txt";
    mergeFiles(update_dir1, update_dir2, check_dir);

    // Phrase1.Setup
    cout<<"---|Phrase1:Setup|---"<<endl;
    FPKS2EClient c;
    FPKS2EServer s;

    c.setup();
    s.setup();

    // Phrase2.Update1
    cout<<"---|Phrase2:Update1|---"<<endl;
    vector<vector<unsigned char>> op = {{'0'}, {'0'}}; // client分为2次批量更新add,add

    batchupdate(c, s, update_dir1, op[0], 1);
    batchupdate(c, s, update_dir2, op[1], 1);

    // Phrase3.Search1
    cout<<"---|Phrase3:Search1|---"<<endl;
    vector<vector<unsigned char>> Wq = c.gen_set_query(c.W, c.size_query);

    map<vector<unsigned char>, vector<vector<unsigned char>>> Ss;

    for(auto w:Wq) {
        vector<unsigned char> L;
        vector<unsigned char> ks;

        c.trapdoor1(w, L, ks);
        
        vector<vector<unsigned char>> S= s.search(L, ks);

        Ss[w] = S;
    }

    c.batch_dec(Ss);

    map<vector<unsigned char>,vector<vector<unsigned char>>> result=c.search_result;

    c.search_output(result, "result/search1_output.txt");

    // Phrase4.Update2
    cout<<"---|Phrase4:Update2|---"<<endl;
    c.reset_cipher_store();
    s.reset_cipher_store();

    batchupdate(c, s, update_dir1, op[0], 2);
    batchupdate(c, s, update_dir2, op[0], 2);

    // Phrase5.Search2
    cout<<"---|Phrase5:Search2|---"<<endl;
    Wq = c.gen_set_query(c.W, c.size_query);

    Ss.clear();

    for(auto w:Wq) {
        vector<unsigned char> L;
        vector<unsigned char> ks;

        c.trapdoor2(w, L, ks);
        
        vector<vector<unsigned char>> S= s.search(L, ks);

        Ss[w] = S;
    }

    c.batch_dec(Ss);

    result=c.search_result;

    c.search_output(result, "result/search2_output.txt");


    cout<<"---|Simple-KS2E Protocol Finished|---"<<endl;

    auto end_time = getCurrentTime();
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Total Execution time: " << elapsed_time.count() << " s" << std::endl;
}