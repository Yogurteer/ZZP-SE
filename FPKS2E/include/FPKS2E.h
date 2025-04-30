#ifndef FPKS2E_H
#define FPKS2E_H

#include "utils.h"
#include "encrypt.h"

# define f_len 8
# define key_len 32

using namespace std;

class FPKS2EClient {
public:

    // key
    static const size_t keylen = key_len;
    char k1[keylen];
    char k2[keylen];

    // state
    map<vector<unsigned char>,pair<vector<unsigned char>,vector<unsigned char>>> lastID; // 记录每个w的上一次链尾的id_op
    map<vector<unsigned char>,unsigned int> SC; // 记录每个w的查询批次
    map<vector<unsigned char>,unsigned int> UC; // 记录每个w的更新批次
    set<vector<unsigned char>> W; // 记录出现过的W空间

    // query
    uint size_query = 3; //单次查询的关键字个数
    map<vector<unsigned char>,vector<vector<unsigned char>>> search_result;

    // client cipher_store中键是L,值是<Iw,Rw,Cw>
    map<vector<unsigned char>, vector<vector<unsigned char>>> cipher_store;

    FPKS2EClient()= default;
    ~FPKS2EClient()= default;

    void setup();

    void init_key(const char key1[keylen], const char key2[keylen]);

    void reset_state();

    /**
     * @brief reset cipher_store
     * 
     */
    void reset_cipher_store();

    /**
     * @brief 加密w_id更新到cipher_store,更新lastID[w]
     * 
     * @param w_id 
     * @param op 
     */
    void update_1(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op, vector<vector<unsigned char>>& Cwid);

    /**
     * @brief 加密w_id更新到cipher_store,更新UC[w]
     * 
     * @param w_id 
     * @param op 
     */
    void update_2(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op, vector<vector<unsigned char>>& Cwid);

    int trapdoor1(vector<unsigned char> w, vector<unsigned char>& L, vector<unsigned char>& ks);

    int trapdoor2(vector<unsigned char> w, vector<unsigned char>& L, vector<unsigned char>& ks);

    vector<vector<unsigned char>> dec_result(vector<vector<unsigned char>> S);

    void batch_dec(map<vector<unsigned char>, vector<vector<unsigned char>>> Ss);

    /**
     * @brief output search result to file
     * 
     * @param search_output_dir 
     */
    void search_output(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, const std::string& search_output_dir);

    /**
     * @brief randomly select size_query keywords from W
     * 
     * @param W 
     * @param size_query 
     * @return vector<string> 
     */
    vector<vector<unsigned char>> gen_set_query(set<vector<unsigned char>> &W, size_t size_query);

    /**
     * @brief randomly gen Rw
     * 
     * @return vector<unsigned char> 
     */
    vector<unsigned char> gen_Rw();
};

class FPKS2EServer {
public:

    // key
    static const size_t keylen = key_len;
    char k1[keylen];
    char k2[keylen];

    // client cipher_store
    map<vector<unsigned char>, vector<vector<unsigned char>>> cipher_store;

    FPKS2EServer()= default;
    ~FPKS2EServer()= default;

    void setup();

    void init_key(const char key1[keylen], const char key2[keylen]);

    /**
     * @brief reset cipher_store
     * 
     */
    void reset_cipher_store();

    void re_update(vector<vector<unsigned char>>& Cwid);

    /**
     * @brief search server
     * 
     * @param L 
     * @param ks 
     * @return vector<vector<unsigned char>> 
     */
    vector<vector<unsigned char>> search(vector<unsigned char> L, vector<unsigned char> ks);


    void copy_cipher_store(map<vector<unsigned char>, vector<vector<unsigned char>>> store_in);
};

#endif // FPKS2E_H