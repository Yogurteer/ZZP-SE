#pragma once

#include "utils.h"
#include "encrypt.h"
#include "DBOGaccess.hpp"

# define f_len 16
# define key_len 32

using namespace std;

struct cipher{
    vector<unsigned char> Iw;
    vector<unsigned char> Rw;
    vector<unsigned char> Cw;
    vector<unsigned char> Iid;
    vector<unsigned char> Rid;
    vector<unsigned char> Cid;
};

class KS2EClient {
public:
    struct state{
        map<vector<unsigned char>,vector<unsigned char>> lastID; // 记录每个w的上一次链尾的id
        map<vector<unsigned char>,vector<unsigned char>> lastW; // 记录每个id的上一次链尾的w
        // map<vector<unsigned char>,unsigned int> SC; // 记录每个w的查询批次
        // map<vector<unsigned char>,unsigned int> UC; // 记录每个w的更新批次
        set<vector<unsigned char>> W; // 记录出现过的w空间
        set<vector<unsigned char>> ID; // 记录出现过的id空间
        set<pair<vector<unsigned char>,vector<unsigned char>>> W_ID; // 记录出现过的w||id空间
    };
    state st;

    // key
    static const size_t keylen = key_len;
    vector<unsigned char> k1;
    vector<unsigned char> k2;

    // query
    uint size_query = 3; //单次查询的关键字个数
    map<vector<unsigned char>,vector<vector<unsigned char>>> search_result;

    // cipher
    map<vector<unsigned char>, cipher> cipher_store;

    KS2EClient() = default;
    ~KS2EClient() = default;

    void setup();

    void copy_key(const char key1[keylen], const char key2[keylen]);

    void reset_state();
    
    void reset_cipher_store();

    void update(vector<unsigned char> w, vector<unsigned char> id, vector<unsigned char> op,
    vector<vector<unsigned char>>& tokens);

    // 默认按关键字搜索
    void search(vector<unsigned char> w, vector<vector<unsigned char>>& request);

    void search_token_id(vector<unsigned char> id, vector<vector<unsigned char>>& request);

    void callback(vector<vector<unsigned char>> param, vector<vector<unsigned char>> response, vector<vector<unsigned char>>& result);

    vector<vector<unsigned char>> dec_result_id(vector<unsigned char> id, vector<vector<unsigned char>> S);

    vector<vector<unsigned char>> gen_set_query(set<vector<unsigned char>> &W, size_t size_query);

    vector<unsigned char> gen_Rw(size_t len);
};

class KS2EOwner : public KS2EClient {
    public:
        using KS2EClient::KS2EClient;
    
        void sharetoken_id(vector<unsigned char> id, vector<vector<unsigned char>>& Pid);
        void sharetoken_w(vector<unsigned char> w, vector<vector<unsigned char>>& Pw);

    };

class KS2EUser : public KS2EClient {
public:
    using KS2EClient::KS2EClient; // 继承构造函数

    void share_w_1(vector<vector<unsigned char>> Pw, vector<vector<unsigned char>>& Dw); 

    void share_w_2(vector<vector<unsigned char>> Pw, vector<vector<unsigned char>> S, 
        vector<vector<vector<unsigned char>>> &S_1, map<vector<unsigned char>, vector<vector<unsigned char>>>& result);

    void share_id_1(vector<vector<unsigned char>> Pid, vector<vector<unsigned char>>& Did);

    void share_id_2(vector<vector<unsigned char>> Pid, vector<vector<unsigned char>> S, 
        vector<vector<vector<unsigned char>>> &S_1, map<vector<unsigned char>, vector<vector<unsigned char>>>& result);
};

class KS2EServer {
public:
    // key
    static const size_t keylen = key_len;
    char k1[keylen];
    char k2[keylen];

    // client cipher_store
    map<vector<unsigned char>, cipher> cipher_store_u;
    map<vector<unsigned char>, cipher> cipher_store_o;

    KS2EServer()= default;
    ~KS2EServer()= default;

    void setup();

    void copy_key(const char key1[keylen], const char key2[keylen]);

    void reset_cipher_store();

    void update(vector<vector<unsigned char>> tokens, DBOGaccess& db1);

    void re_share_w_1(vector<vector<unsigned char>> Dw, vector<vector<unsigned char>>& S, DBOGaccess& db1);

    void re_share_w_2(vector<vector<vector<unsigned char>>> &S_1, DBOGaccess& db1);

    void re_share_id_1(vector<vector<unsigned char>> Did, vector<vector<unsigned char>>& S, DBOGaccess& db1);

    void re_share_id_2(vector<vector<vector<unsigned char>>> &S_1, DBOGaccess& db1);

    void search(string type, vector<vector<unsigned char>> request, vector<vector<unsigned char>>& response, DBOGaccess& db1);

    void search_id(string type, vector<vector<unsigned char>> request, vector<vector<unsigned char>>& S, DBOGaccess& db1);
};

