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
void update_1(pair<vector<unsigned char>,
              vector<unsigned char>> w_id, 
              vector<unsigned char> op, 
              vector<vector<unsigned char>>& Cwid);
````

* `function`：更新<w,id>到cipher_store,以lastID[w]记录链尾的id||op,初始为$\perp $
* `w_id `：更新数据文件目录
* `op`：更新操作类型,0->add,1->del
* `Cwid`： 更新密文索引Cwid = {L, Iw, Rw, Cw}需发送给server

````c++
void update_2(pair<vector<unsigned char>,
              vector<unsigned char>> w_id, 
              vector<unsigned char> op, 
              vector<vector<unsigned char>>& Cwid);
````

* `function`：更新<w,id>到cipher_store,以UC[w]记录链尾的更新次数uw,初始为0
* `w_id `：更新数据文件目录
* `op`：更新操作类型,0->add,1->del
* `Cwid`： 更新密文索引Cwid = {L, Iw, Rw, Cw}需发送给server

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

#### re_update

```c++
void re_update(vector<vector<unsigned char>>& Cwid);
```

- `function`：响应client的更新，将接收的密文索引更新到cipher_store
- `Cwid`：接收client发来的密文索引

#### search

```cpp
vector<vector<unsigned char>> search(vector<unsigned char> L, vector<unsigned char> ks);
```

- `function`：使用令牌L和ks查询，返回值即为响应结果，通常记为S
- `L`：搜索令牌，记录tokens在cipher_store中的索引
- `ks`：搜索令牌，解密密钥

## FPKS2E调用demo

```c++
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
```
