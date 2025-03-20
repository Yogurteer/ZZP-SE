#ifndef FPKS2E_H
#define FPKS2E_H

#include "utils.h"
#include "encrypt.h"
#include "com.h"

# define f_len 8

using namespace std;

class FPKS2E {
public:

    // key
    static const size_t keylen = 32;
    char k1[keylen];
    char k2[keylen];

    // state
    map<string,pair<unsigned int,char>> lastID; // 记录每个w的上一次链尾的id_op
    map<string,int> SC; // 记录每个w的查询批次
    set<string> W; // 记录出现过的W空间

    // client
    uint size_search = 3; //单次查询的关键字个数
    map<string,vector<unsigned int>> search_result;

    // server EDB中键是L,值是<Iw,Rw,Cw>
    map<vector<unsigned char>, vector<vector<unsigned char>>> EDB;

    FPKS2E();
    ~FPKS2E();

    /**
     * @brief initial key
     * 
     */
    void setup();

    /**
     * @brief 倒排索引加密w_id更新到EDB
     * 
     * @param w_id 
     * @param op 
     */
    void update(pair<string,unsigned int> w_id, char op);

    /**
     * @brief batch update
     * 
     * @param update_dir 
     * @param op 
     */
    void batchupdate(char *update_dir, char op);

    /**
     * @brief 从链尾开始查询w
     * 
     * @param w 
     * @return vector<unsigned int> 
     */
    vector<unsigned int> search(string w);

    /**
     * @brief batch search
     * 
     * @param Wq 
     * @return map<string,vector<unsigned int>> 
     */
    map<string,vector<unsigned int>> batchsearch(vector<string> Wq);

    /**
     * @brief output search result to file
     * 
     * @param search_output_dir 
     */
    void search_output(map<string,vector<unsigned int>> search_result, const std::string& search_output_dir);

    /**
     * @brief randomly select size_query keywords from W
     * 
     * @param W 
     * @param size_query 
     * @return vector<string> 
     */
    vector<string> gen_set_query(set<string> &W, size_t size_query);

    /**
     * @brief randomly gen Rw
     * 
     * @return vector<unsigned char> 
     */
    vector<unsigned char> gen_Rw();
};

#endif