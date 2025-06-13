#include "inter.h"

void fullupdate(KS2EOwner& o, KS2EServer& s, 
    pair<vector<unsigned char>, vector<unsigned char>> w_id, vector<unsigned char> op, DBOGaccess& db1)
{
    if(op!= vector<unsigned char>{'0'}){
        throw runtime_error("Error: current version op only support add!");
    }

    // owner update
    pair<vector<unsigned char>, cipher> Cwid;
    o.update_1(w_id, op, Cwid);

    // server re update
    s.re_o_update(Cwid, db1);
}

void batchupdate(KS2EOwner &o, KS2EServer &s, char *update_dir, vector<unsigned char> op, DBOGaccess& db1)
{
    std::cout << "Updating batch data..." << std::endl;

    // 从文件读入DB
    map<string, vector<string>> Ulist;// {w,{id}}
    Ulist.clear(); // 清空Ulist
    cout << "Update DB source dir: " << update_dir << endl;
    read_keywords(update_dir, Ulist);

    // 遍历Ulist中的每个关键字
    for (const auto& entry : Ulist) {
        string w = entry.first;
        vector<unsigned char> v_w(w.begin(), w.end());// 将关键字转换为字节向量
        vector<string> ids = entry.second;      
        for(auto &id:ids){
            vector<unsigned char> v_id(id.begin(), id.end());// 将id转换为字节向量
            pair w_id = make_pair(v_w, v_id);
            fullupdate(o, s, w_id, op, db1);
        }
    }
}

void fullshare_id(KS2EUser &u, KS2EServer &s, vector<vector<unsigned char>> &Pid, DBOGaccess& db1)
{
    // user share 1 parse
    vector<vector<unsigned char>> Did;
    u.share_id_1(Pid, Did);
    // server re share 1
    vector<vector<unsigned char>> S;
    s.re_share_id_1(Did, S, db1);
    // user share 2 gen cipher
    vector<vector<vector<unsigned char>>> S_1;
    map<vector<unsigned char>, vector<vector<unsigned char>>> u_share_id_result;
    u.share_id_2(Pid, S, S_1, u_share_id_result);
    search_output_vk(u_share_id_result, share_id_dir, db1);
    // server re share 2
    s.re_share_id_2(S_1, db1);
}

void fullshare_w(KS2EUser &u, KS2EServer &s, vector<vector<unsigned char>> &Pw, DBOGaccess& db1)
{
    // user share 1 parse
    vector<vector<unsigned char>> Dw;
    u.share_w_1(Pw, Dw);
    // server re share 1
    vector<vector<unsigned char>> S;
    s.re_share_w_1(Dw, S, db1);
    // user share 2 gen cipher
    vector<vector<vector<unsigned char>>> S_1;
    map<vector<unsigned char>, vector<vector<unsigned char>>> u_share_w_result;
    u.share_w_2(Pw, S, S_1, u_share_w_result);
    search_output_kv(u_share_w_result, share_w_dir, db1);
    // server re share 2
    s.re_share_w_2(S_1, db1);
}

void fullsearch_id(string client_type, KS2EClient &c, KS2EServer &s, vector<unsigned char> id, vector<vector<unsigned char>>& getws, DBOGaccess& db1)
{
    // user gen trapdoor for id
    vector<vector<unsigned char>> Tid;
    c.trapdoor_id(id, Tid);// send Tid to server
    // server excute search
    vector<vector<unsigned char>> S;
    s.search_id(client_type, Tid, S, db1);
    // user decode
    vector<vector<unsigned char>> result;
    getws = c.dec_result_id(id, S);
}

void fullsearch_w(string client_type, KS2EClient &c, KS2EServer &s, vector<unsigned char> w, vector<vector<unsigned char>>& getids, DBOGaccess& db1)
{
    // user gen trapdoor for id
    vector<vector<unsigned char>> Tw;
    c.trapdoor_w(w, Tw);// send Tid to server

    // server excute search
    vector<vector<unsigned char>> S;
    s.search_w(client_type, Tw, S, db1);

    // user decode
    vector<vector<unsigned char>> result;
    getids = c.dec_result_w(w, S);
}

void batchsearch_id(string client_type, KS2EClient &c, KS2EServer &s, vector<vector<unsigned char>> IDq, 
    map<vector<unsigned char>, vector<vector<unsigned char>>> &result, DBOGaccess& db1){

    result.clear();

    vector<vector<unsigned char>> getws;
    
    for(auto id:IDq) {
        fullsearch_id(client_type, c, s, id, getws, db1);
        result[id] = getws;
        getws.clear();
    }
}

void batchsearch_w(string client_type, KS2EClient &c, KS2EServer &s, vector<vector<unsigned char>> Wq, 
    map<vector<unsigned char>, vector<vector<unsigned char>>> &result, DBOGaccess& db1){

    result.clear();

    vector<vector<unsigned char>> getids;
    
    for(auto w:Wq) {
        fullsearch_w(client_type, c, s, w, getids, db1);
        result[w] = getids;
        getids.clear();
    }
}
