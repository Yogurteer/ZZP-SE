#include "KS2E.h"
#include "utils.h"
#include "inter.h"

using namespace std;

int main() {

    cout<<"---|KS2E Protocol Test Processing|---"<<endl;

    auto start_time = getCurrentTime();

    // DB dir
    char update_dir1[100] = "dataset/min_shieldDB/Ulist1.txt";
    char update_dir2[100] = "dataset/min_shieldDB/Ulist2.txt";

    string check_dir = "dataset/view/Ulist1p2.txt";// view correctness
    mergeFiles(update_dir1, update_dir2, check_dir);

    // Phrase1.Setup
    cout<<"---|Phrase1:Setup|---"<<endl;

    KS2EUser u;
    KS2EOwner o;
    KS2EServer s;

    u.setup();
    o.setup();
    s.setup();

    // Phrase2.Update1
    cout<<"---|Phrase2:Update1|---"<<endl;

    vector<vector<unsigned char>> op = {{'0'}, {'0'}}; // client分为2次批量更新add,add

    batchupdate(o, s, update_dir1, op[0]);
    batchupdate(o, s, update_dir2, op[1]);

    // Phrase3.Sharetoken1-id
    cout<<"---|Phrase3:Sharetoken|---"<<endl;

    vector<unsigned char> share_id = o.gen_set_query(o.st.ID, 1)[0];

    string s_id(share_id.begin(), share_id.end());

    cout<< "Share id: "<<s_id<<endl;
 
    vector<vector<unsigned char>> Pid;

    o.sharetoken_id(share_id, Pid);// assume u get Pid

    vector<unsigned char> share_w = o.gen_set_query(o.st.W, 1)[0];

    string s_w(share_w.begin(), share_w.end());

    cout<< "Share w: "<<s_w<<endl;

    vector<vector<unsigned char>> Pw;

    o.sharetoken_w(share_w, Pw);// assume u get Pw

    // Phrase4.Sharet
    cout<<"---|Phrase4:Share|---"<<endl;

    fullshare_id(u, s, Pid);

    fullshare_w(u, s, Pw);

    // Phrase5.Search1-w    
    cout<<"---|Phrase5:Search1-w|---"<<endl;

    vector<vector<unsigned char>> Wq = u.gen_set_query(u.st.W, 1);

    string s_w2(Wq[0].begin(), Wq[0].end());

    cout<< "Search w: "<<s_w<<endl;
    
    map<vector<unsigned char>, vector<vector<unsigned char>>> result;

    // batchsearch_id("user", u, s, IDq, result);
    batchsearch_w("user", u, s, Wq, result);

    u.search_result = result;

    string s1_id_dir = "result/search1_w_output.txt";
    search_output(u.search_result, s1_id_dir);

    // Phrase6.Search2-id    
    cout<<"---|Phrase6:Search2-id|---"<<endl;
    vector<vector<unsigned char>> IDq = u.gen_set_query(u.st.ID, 1);

    string s_id2(IDq[0].begin(), IDq[0].end());

    cout<< "Search id: "<<s_id2<<endl;

    batchsearch_id("user", u, s, IDq, result);

    u.search_result = result;

    string s2_id_dir = "result/search2_id_output.txt";
    search_output(u.search_result, s2_id_dir);

    cout<<"---|KS2E Protocol Test Finished|---"<<endl;

    auto end_time = getCurrentTime();
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Total Execution time: " << elapsed_time.count() << " s" << std::endl;
}