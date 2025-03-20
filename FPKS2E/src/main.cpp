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
    FPKS2E fp;
    fp.setup();

    // Phrase2.Update
    cout<<"---|Phrase2:Update|---"<<endl;
    char op[2] = {'0','0'};// client分为2次批量更新add,add
    fp.batchupdate(update_dir1,op[0]);
    fp.batchupdate(update_dir2,op[1]);

    // Phrase3.Search
    cout<<"---|Phrase3:Search|---"<<endl;
    vector<string> Wq = fp.gen_set_query(fp.W, fp.size_search);
    map<string,vector<unsigned int>> search_result = fp.batchsearch(Wq);
    fp.search_output(search_result, "result/search_output.txt");

    cout<<"---|Simple-KS2E Protocol Finished|---"<<endl;

    auto end_time = getCurrentTime();
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Total Execution time: " << elapsed_time.count() << " s" << std::endl;
}