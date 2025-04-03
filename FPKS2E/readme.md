# Forward Privacy in KS2E

## Version

v3 on 3.27 

by Zhenpeng Zhou

## **Description**

- 通过GMssl实现hmac
- 前向安全相关函数封装在类FPKS2EClient和FPKS2EServer中
- 所有输入输出数据以`vector<unsigned char>`类型传递,如keyword,id,op
- 包含以lastID[w]和以UC[w]记录链尾的2个版本

## FPKS2EClient

### 成员变量

- keylen：密钥长度
- k1：密钥K1
- k2：密钥K2
- lastID[]：记录关键字下最后一个<id,op>
- SC[]：记录关键字查询次数
- UC[]：记录关键字更新次数
- W[]：记录client更新过的所有关键字
- size_query：client单次批量查询的关键字个数
- search_result：存储查询结果
- cipher_store：存储加密数据

### 主要成员函数

#### init_key

````c++
void init_key(const char key1[keylen], const char key2[keylen]);
````

* `function`：初始化密钥
* `key1[keylen]`：密钥1
* `key2[keylen]`：密钥2

#### update

````c++
void update_1(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op);
````

* `function`：更新<w,id>到cipher_store,以lastID[w]记录链尾的id||op,初始为$\perp $
* `w_id `：更新数据文件目录
* `op`：更新操作类型,0->add,1->del

````c++
void update_2(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op);
````

* `function`：更新<w,id>到cipher_store,以UC[w]记录链尾的更新次数uw,初始为0
* `w_id `：更新数据文件目录
* `op`：更新操作类型,0->add,1->del

#### trapdoor

```c++
int trapdoor1(vector<unsigned char> w, vector<unsigned char>& L, vector<unsigned char>& ks);
```

* `function`：为关键字w生成搜索令牌L和ks，lastID[]版本，返回0表示success，1表示fail
* `w `：查询关键字
* `L`：搜索令牌，记录tokens在cipher_store中的索引
* `ks`：搜索令牌，解密密钥

```c++
int trapdoor2(vector<unsigned char> w, vector<unsigned char>& L, vector<unsigned char>& ks);
```

* `function`：为关键字w生成搜索令牌L和ks，UC[]版本，返回0表示success，1表示fail
* `w `：查询关键字
* `L`：搜索令牌，记录tokens在cipher_store中的索引
* `ks`：搜索令牌，解密密钥

#### decrypt

```cpp
vector<vector<unsigned char>> dec_result(vector<vector<unsigned char>> S);
```

- `function`：解密查询结果得到明文id集合
- `S`：server发来的查询响应结果，内容为1个w下对应的多个id

## FPKS2EServer

### 成员变量

- keylen：密钥长度
- k1：密钥K1
- k2：密钥K2
- cipher_store：存储加密数据

### 主要成员函数

#### init_key

````c++
void init_key(const char key1[keylen], const char key2[keylen]);void setup();
````

* `function`：初始化密钥
* `key1[keylen]`：密钥1
* `key2[keylen]`：密钥2

#### search

```cpp
vector<vector<unsigned char>> search(vector<unsigned char> L, vector<unsigned char> ks);
```

- `function`：使用令牌L和ks查询，返回值即为响应结果，通常记为S
- `L`：搜索令牌，记录tokens在cipher_store中的索引
- `ks`：搜索令牌，解密密钥

## FPKS2E调用demo

```c++
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
```
