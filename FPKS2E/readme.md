## Forward Privacy in KS2E

**Description**

通过GMssl实现hmac,前向安全相关函数封装在类FPKS2E中

## FPKS2E成员变量

- keylen：密钥长度
- k1：密钥K1
- k2：密钥K2
- lastID[]：记录关键字下最后一个<id,op>
- SC[]：记录关键字查询次数
- W[]：记录client更新过的所有关键字
- size_search：client单次批量查询的关键字个数
- search_result：存储单次批量查询结果
- EDB：server中的加密数据

## FPKS2E主要成员函数

##### Setup

````c++
void setup();
````

* `function`：初始化密钥

##### Update

````c++
void update(pair<string,unsigned int> w_id, char op);
````

* `function`：更新<w,id>到EDB
*  `w_id `：更新数据文件目录
* `op`：更新操作类型,0->add,1->del

**Search**

```c++
vector<unsigned int> search(string w);
```

* `function`：从EDB中查询关键字w
*  `w `：查询关键字

## FPKS2E调用demo

```c++
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
```






