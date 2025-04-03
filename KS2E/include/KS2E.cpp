#include "KS2E.h"
#include <iostream>

using namespace std;

void KS2EClient::setup() {
    std::cout << "Setting up KS2E Client..." << std::endl;

    unsigned char c_k1[keylen];
    unsigned char c_k2[keylen];

    RAND_bytes(c_k1, key_len);
    RAND_bytes(c_k2, key_len);

    this->k1 = toByteVector(c_k1, key_len);
    this->k2 = toByteVector(c_k2, key_len);

    // k1 = std::vector<unsigned char>(32, '\1');
    // k2 = std::vector<unsigned char>(32, '\2');

}

void KS2EClient::copy_key(const char key1[keylen], const char key2[keylen])
{

}

void KS2EClient::reset_state()
{
    st.lastID.clear();
    st.lastW.clear();
    st.W.clear();
}

void KS2EClient::reset_cipher_store()
{
    this->cipher_store.clear();
}

void KS2EClient::update_1(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op,
    pair<vector<unsigned char>, cipher>& Cwid){
    
    vector<unsigned char> w=w_id.first;
    vector<unsigned char> id=w_id.second;

    // escape existing w||id
    if(this->st.W_ID.find(w_id)!=this->st.W_ID.end()){
        cout<<"w: " ;view_pt(w);cout<<" id: " ;view_pt(id);cout<<endl;
        cout<<"Continue: current w id exist in client,not repeat update!"<<endl;
        return;
    }
  
    // init w' id'
    vector<unsigned char> w_1={'\0'};
    vector<unsigned char> id_1={'\0'};

    if(st.lastID.find(w)!=st.lastID.end()){
        id_1=st.lastID[w];
    }

    if(st.lastW.find(id)!=st.lastW.end()){
        w_1=st.lastW[id];
    }

    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    // client插入w,id,w||id,用于生成查询，排除重复w||id
    this->st.W.insert(w);
    this->st.ID.insert(id);
    this->st.W_ID.insert(w_id);

    // 计算中间量L,Rw,Rid,kw,kid,Cw,Cid
    auto v_w_id = concatVectors(w, id);
    auto v_id_w = concatVectors(id, w);
    vector<unsigned char> L = gm_hmac(v_w_id.data(),v_w_id.size(),k1.data(),k1.size(),f_len); // F即sha256_hmac

    vector<unsigned char> Rw = gen_Rw(f_len); 

    vector<unsigned char> Rid = gen_Rw(f_len);

    vector<unsigned char> kw = gm_hmac(w.data(),w.size(),k2.data(),k2.size(),f_len);

    vector<unsigned char> kid = gm_hmac(id.data(),id.size(),k2.data(),k2.size(),f_len);

    vector<unsigned char> Cw = my_xor(id, kw);

    vector<unsigned char> Cid = my_xor(w, kid);

    // 计算Iw
    vector<unsigned char> Iw;
    vector<unsigned char> Jw = gm_hmac(v_w_id.data(),v_w_id.size(),k2.data(),k2.size(),f_len);
    vector<unsigned char> HFR1 = gm_hmac(Jw.data(),Jw.size(),Rw.data(),Rw.size(),f_len*2);// F(Jw,Rw)

    if (id_1 == vector<unsigned char>{'\0'}){// id_1为空        
        vector<unsigned char> zero_head(f_len*2, 0);
        Iw = my_xor(HFR1, zero_head);
    }
    else{// id_1不为空
        auto v_w_id_1 = concatVectors(w, id_1);
        vector<unsigned char> L_1 = gm_hmac(v_w_id_1.data(),v_w_id_1.size(),k1.data(),k1.size(),f_len);
        vector<unsigned char> Jw_1 = gm_hmac(v_w_id_1.data(),v_w_id_1.size(),k2.data(),k2.size(),f_len);
        vector<unsigned char> L_1_Jw_1 = concatVectors(L_1, Jw_1);
        Iw = my_xor(HFR1, L_1_Jw_1);
    }

    // 计算Iid
    vector<unsigned char> Iid;
    vector<unsigned char> Jid = gm_hmac(v_id_w.data(),v_id_w.size(),k2.data(),k2.size(),f_len);// F(k2,id||w)
    vector<unsigned char> HJR = gm_hmac(Jid.data(),Jid.size(),Rid.data(),Rid.size(),f_len*2);// H(Jid,Rid)
    
    if (w_1 == vector<unsigned char>{'\0'}){// w_1为空
        vector<unsigned char> zero_head(f_len*2, 0);
        Iid = my_xor(zero_head, HJR);
    }
    else{// w_1不为空
        auto v_w_1_id = concatVectors(w_1, id);
        auto v_id_w_1 = concatVectors(id, w_1);
        vector<unsigned char> L_1 = gm_hmac(v_w_1_id.data(),v_w_1_id.size(),k1.data(),k1.size(),f_len);
        vector<unsigned char> Jid_1 = gm_hmac(v_id_w_1.data(),v_id_w_1.size(),k2.data(),k2.size(),f_len);
        vector<unsigned char> L_1_Jid_1 = concatVectors(L_1, Jid_1);
        Iid = my_xor(L_1_Jid_1, HJR);
    }

    // update st
    st.lastW[id] = w;
    st.lastID[w] = id;

    // client将Cwid存入cipher_store,理论上把Cwid发给server
    Cwid = make_pair(L, cipher{Iw, Rw, Cw, Iid, Rid, Cid});
}

int KS2EClient::trapdoor_w(vector<unsigned char> w, vector<vector<unsigned char>>& Tw)
{
    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    if (st.lastID.find(w) == st.lastID.end()) {
        string wstring(w.begin(),w.end());
        throw runtime_error("trapdoor_w gen failed,lastID["+wstring+"] is empty!");
    }

    vector<unsigned char> id = st.lastID[w];

    auto v_w_id = concatVectors(w, id);
    vector<unsigned char> L = gm_hmac(v_w_id.data(),v_w_id.size(),k1.data(),k1.size(),f_len);
    vector<unsigned char> Jw = gm_hmac(v_w_id.data(),v_w_id.size(),k2.data(),k2.size(),f_len);
    
    Tw.clear();
    Tw.push_back(L);
    Tw.push_back(Jw);

    return 1;
}

int KS2EClient::trapdoor_id(vector<unsigned char> id, vector<vector<unsigned char>>& Tid)
{
    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    if (st.lastW.find(id) == st.lastW.end()) {
        string idstring(id.begin(),id.end());
        throw runtime_error("trapdoor_id gen failed,lastW["+idstring+"] is empty!");
    }

    vector<unsigned char> w = st.lastW[id];

    auto v_w_id = concatVectors(w, id);
    auto v_id_w = concatVectors(id, w);
    vector<unsigned char> L = gm_hmac(v_w_id.data(),v_w_id.size(),k1.data(),k1.size(),f_len);
    vector<unsigned char> Jid = gm_hmac(v_id_w.data(),v_id_w.size(),k2.data(),k2.size(),f_len);
    
    Tid.clear();
    Tid.push_back(L);
    Tid.push_back(Jid);

    return 1;
}

vector<vector<unsigned char>> KS2EClient::dec_result_w(vector<unsigned char> w, vector<vector<unsigned char>> S)
{
    vector<vector<unsigned char>> getids;

    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    vector<unsigned char> kw = gm_hmac(w.data(),w.size(),k2.data(),k2.size(),f_len);

    for(auto &Cw:S){
        vector<unsigned char> id = my_xor(Cw, kw);

        getids.push_back(id);
    }

    return getids;// 返回id集合,字节向量版本
}

vector<vector<unsigned char>> KS2EClient::dec_result_id(vector<unsigned char> id, vector<vector<unsigned char>> S)
{
    vector<vector<unsigned char>> getws;

    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    vector<unsigned char> kid = gm_hmac(id.data(),id.size(),k2.data(),k2.size(),f_len);

    for(auto &Cid:S){
        vector<unsigned char> w = my_xor(Cid, kid);

        getws.push_back(w);
    }

    return getws;// 返回id集合,字节向量版本
}

vector<vector<unsigned char>> KS2EClient::gen_set_query(set<vector<unsigned char>> &W, size_t size_query){
    
    if(size_query>W.size()){
        throw runtime_error("查询数量大于对象空间,请修改查询数量!");
    }

    vector<vector<unsigned char>> set_query;

    // 1. 将 set<string> 转换为 vector<string>，方便进行随机选择
    std::vector<vector<unsigned char>> temp(W.begin(), W.end());

    // 2. 使用随机数生成器来打乱 vector<string>
    std::random_device rd;
    std::mt19937 g(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())); // 使用当前时间作为种子
    std::shuffle(temp.begin(), temp.end(), g);

    // 3. 取前 size_query 个元素填充 set_query
    for (size_t i = 0; i < size_query; ++i) {
        set_query.push_back(temp[i]);
    }

    return set_query;
}

vector<unsigned char> KS2EClient::gen_Rw(size_t len) {
    // 初始化随机数生成器
    std::srand(std::time(0));  // 用当前时间作为种子

    // 创建长度为4的vector
    std::vector<unsigned char> Rw(len);

    // 填充vector，生成0到255之间的随机数
    for (size_t i = 0; i < len; ++i) {
        Rw[i] = std::rand() % 256;  // 生成0到255之间的随机数
    }

    return Rw;
}

void KS2EOwner::sharetoken_id(vector<unsigned char> id, vector<vector<unsigned char>> &Pid)
{
    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;
    
    if (st.lastW.find(id) == st.lastW.end()) {
        throw runtime_error("sharetoken_id failed,lastW[] is empty!");
    }

    vector<unsigned char> w = st.lastW[id];
    
    auto v_w_id = concatVectors(w, id);
    auto v_id_w = concatVectors(id, w);
    vector<unsigned char> L = gm_hmac(v_w_id.data(),v_w_id.size(),k1.data(),k1.size(),f_len);
    vector<unsigned char> Jid = gm_hmac(v_id_w.data(),v_id_w.size(),k2.data(),k2.size(),f_len);
    vector<unsigned char> kid = gm_hmac(id.data(),id.size(),k2.data(),k2.size(),f_len);

    Pid.clear();
    Pid = {L, Jid, id, kid};
}

void KS2EOwner::sharetoken_w(vector<unsigned char> w, vector<vector<unsigned char>> &Pw)
{
    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;
    
    if(st.lastID.find(w)==st.lastID.end()){
        throw runtime_error("sharetoken_w failed,lastID[] is empty!");
    }

    vector<unsigned char> id = st.lastID[w];

    auto v_w_id = concatVectors(w, id);
    auto v_id_w = concatVectors(id, w);
    vector<unsigned char> L = gm_hmac(v_w_id.data(),v_w_id.size(),k1.data(),k1.size(),f_len);
    vector<unsigned char> Jw = gm_hmac(v_w_id.data(),v_w_id.size(),k2.data(),k2.size(),f_len);
    vector<unsigned char> kw = gm_hmac(w.data(),w.size(),k2.data(),k2.size(),f_len);

    Pw.clear();
    Pw= {L, Jw, w, kw};
}

void KS2EUser::share_w_1(vector<vector<unsigned char>> Pw, vector<vector<unsigned char>>& Dw)
{
    Dw.clear();
    Dw.push_back(Pw[0]); // L
    Dw.push_back(Pw[1]); // Jw
}

void KS2EUser::share_w_2(vector<vector<unsigned char>> Pw, vector<vector<unsigned char>> S, 
    map<vector<unsigned char>, cipher> &S_1, map<vector<unsigned char>, vector<vector<unsigned char>>>& result)
{
    S_1.clear();

    vector<unsigned char> w = Pw[2];
    vector<unsigned char> kw = Pw[3];

    this->st.W.insert(w);
    
    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    vector<unsigned char> id;
    
    for(auto &s:S){
        // s = Cw
        id = my_xor(s, kw);

        pair<vector<unsigned char>, cipher> Cidw;

        pair w_id = make_pair(w, id);
        this->update_1(w_id, {'0'}, Cidw);
        result[w].push_back(id);
        S_1[Cidw.first] = Cidw.second;
    }
}

void KS2EUser::share_id_1(vector<vector<unsigned char>> Pid, vector<vector<unsigned char>>& Did)
{
    Did.clear();
    Did.push_back(Pid[0]); // L
    Did.push_back(Pid[1]); // Jid
}

void KS2EUser::share_id_2(vector<vector<unsigned char>> Pid, vector<vector<unsigned char>> S, 
    map<vector<unsigned char>, cipher> &S_1, map<vector<unsigned char>, vector<vector<unsigned char>>>& result)
{
    S_1.clear();

    vector<unsigned char> id = Pid[2];
    vector<unsigned char> kid = Pid[3];

    this->st.ID.insert(id);
    
    vector<unsigned char> k1 = this->k1;
    vector<unsigned char> k2 = this->k2;

    vector<unsigned char> w;
    
    for(auto &s:S){
        // s = Cid
        w = my_xor(s, kid);

        pair<vector<unsigned char>, cipher> Cwid;

        pair w_id = make_pair(w, id);
        this->update_1(w_id, {'0'}, Cwid);
        result[id].push_back(w);
        S_1[Cwid.first] = Cwid.second;
    }
}

void KS2EServer::setup()
{
    std::cout << "Setting up KS2E Server..." << std::endl;
}

void KS2EServer::copy_key(const char key1[keylen], const char key2[keylen])
{

}

void KS2EServer::reset_cipher_store()
{
    this->cipher_store_o.clear();
    this->cipher_store_u.clear();
}

void KS2EServer::re_o_update(pair<vector<unsigned char>, cipher> Cwid)
{
    auto L = Cwid.first;
    auto c = Cwid.second;
    cipher_store_o[L] = c;
}

void KS2EServer::re_share_w_1(vector<vector<unsigned char>> Dw, vector<vector<unsigned char>> &S)
{
    auto L = Dw[0];
    auto Jw = Dw[1];

    S.clear();

    vector<unsigned char> zero_L(f_len, 0); //表示空

    while(L!=zero_L){
        cipher c = this->cipher_store_o.at(L);
        auto Iw = c.Iw;
        auto Rw = c.Rw;
        auto Cw = c.Cw;

        vector<unsigned char> HJR = gm_hmac(Jw.data(),Jw.size(),Rw.data(),Rw.size(),f_len*2);
        vector<unsigned char> L_1_Jw_1 = my_xor(Iw, HJR);
        vector<unsigned char> L_1(L_1_Jw_1.begin(), L_1_Jw_1.begin() + f_len);
        vector<unsigned char> Jw_1(L_1_Jw_1.begin() + f_len, L_1_Jw_1.end());

        S.push_back(Cw);

        L = L_1;
        Jw = Jw_1;
    }
}

void KS2EServer::re_share_w_2(map<vector<unsigned char>, cipher> S_1)
{
    for(auto &pair:S_1){
        auto L = pair.first;
        auto c = pair.second;
        cipher_store_u[L] = c;
    }
}

void KS2EServer::re_share_id_1(vector<vector<unsigned char>> Did, vector<vector<unsigned char>> &S)
{
    auto L = Did[0];
    auto Jid = Did[1];

    S.clear();

    vector<unsigned char> zero_L(f_len, 0); //表示空

    while(L!=zero_L){
        cipher c = this->cipher_store_o.at(L);
        auto Iid = c.Iid;
        auto Rid = c.Rid;
        auto Cid = c.Cid;

        vector<unsigned char> HJR = gm_hmac(Jid.data(),Jid.size(),Rid.data(),Rid.size(),f_len*2);
        vector<unsigned char> L_1_Jid_1 = my_xor(Iid, HJR);
        vector<unsigned char> L_1(L_1_Jid_1.begin(), L_1_Jid_1.begin() + f_len);
        vector<unsigned char> Jid_1(L_1_Jid_1.begin() + f_len, L_1_Jid_1.end());

        S.push_back(Cid);

        L = L_1;
        Jid = Jid_1;
    }
}

void KS2EServer::re_share_id_2(map<vector<unsigned char>, cipher> S_1)
{
    for(auto &pair:S_1){
        auto L = pair.first;
        auto c = pair.second;
        cipher_store_u[L] = c;
    }
}

void KS2EServer::search_w(string type, vector<vector<unsigned char>> Tw, vector<vector<unsigned char>>& S)
{
    map<vector<unsigned char>, cipher> cipher_store;
    
    if(type=="user") cipher_store = this->cipher_store_u;
    else if(type=="owner") cipher_store = this->cipher_store_o;
    else{
        throw runtime_error("search_w failed,type is wrong!");
    }
    
    auto L = Tw[0];
    auto Jw = Tw[1];

    S.clear();

    vector<unsigned char> zero_L(f_len, 0);

    while(L!=zero_L){
        cipher c = cipher_store.at(L);
        auto Iw = c.Iw;
        auto Rw = c.Rw;
        auto Cw = c.Cw;

        vector<unsigned char> HJW = gm_hmac(Jw.data(),Jw.size(),Rw.data(),Rw.size(),f_len*2);// H(Jw,Rw)
        vector<unsigned char> L_1_Jw_1 = my_xor(HJW, Iw);
        vector<unsigned char> L_1(L_1_Jw_1.begin(), L_1_Jw_1.begin() + f_len);
        vector<unsigned char> Jw_1(L_1_Jw_1.begin() + f_len, L_1_Jw_1.end());

        S.push_back(Cw);

        L = L_1;
        Jw = Jw_1;
    }
}

void KS2EServer::search_id(string type, vector<vector<unsigned char>> Tid, vector<vector<unsigned char>>& S)
{
    map<vector<unsigned char>, cipher> cipher_store;
    
    if(type=="user") cipher_store = this->cipher_store_u;
    else if(type=="owner") cipher_store = this->cipher_store_o;
    else {
        throw runtime_error("search_id failed,type is wrong!");
    }
    
    auto L = Tid[0];
    auto Jid = Tid[1];

    S.clear();

    vector<unsigned char> zero_L(f_len, 0);

    while(L!=zero_L){
        cipher c = cipher_store.at(L);
        auto Iid = c.Iid;
        auto Rid = c.Rid;
        auto Cid = c.Cid;

        vector<unsigned char> HJR = gm_hmac(Jid.data(),Jid.size(),Rid.data(),Rid.size(),f_len*2);

        vector<unsigned char> L_1_Jid_1 = my_xor(HJR, Iid);
        vector<unsigned char> L_1(L_1_Jid_1.begin(), L_1_Jid_1.begin() + f_len);
        vector<unsigned char> Jid_1(L_1_Jid_1.begin() + f_len, L_1_Jid_1.end());

        S.push_back(Cid);

        L = L_1;
        Jid = Jid_1;
    }
}
