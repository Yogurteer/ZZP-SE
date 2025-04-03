#include "FPKS2E.h"
#include "utils.h"

using namespace std;

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

    c.batchupdate(update_dir1,op[0],1);
    c.batchupdate(update_dir2,op[1],1);

    s.copy_cipher_store(c.cipher_store);

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

    c.batchupdate(update_dir1,op[0],2);
    c.batchupdate(update_dir2,op[1],2);

    s.copy_cipher_store(c.cipher_store);

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