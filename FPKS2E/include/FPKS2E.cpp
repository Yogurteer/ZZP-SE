#include "FPKS2E.h"
#include <iostream>

void FPKS2EClient::setup() {
    std::cout << "Setting up FPKS2E Client..." << std::endl;
}

void FPKS2EClient::init_key(const char key1[keylen], const char key2[keylen])
{
    memcpy(k1, key1, keylen);
    memcpy(k2, key2, keylen);
}

void FPKS2EClient::reset_state()
{
    lastID.clear();
    W.clear();
    SC.clear();
    UC.clear();
}

void FPKS2EClient::reset_cipher_store()
{
    this->cipher_store.clear();
}

void FPKS2EClient::update_1(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op, vector<vector<unsigned char>>& Cwid){
    vector<unsigned char> w=w_id.first;
    vector<unsigned char> id=w_id.second;

    vector<unsigned char> v_k1 = toByteVector(k1);
    vector<unsigned char> v_k2 = toByteVector(k2);

    // 1.client插入w到W
    this->W.insert(w);

    // 2.判断lastID[w]是否为空,赋值id_op
    pair<vector<unsigned char>,vector<unsigned char>> id_op_1= {{'\0'},{'\0'}}; //默认为空
    if(lastID.find(w)==lastID.end()){
        SC[w]=0;
    }
    else{
        id_op_1=lastID[w]; // 求上一次链尾的id_op_1
    }

    // 3.计算中间量L,Rw,ks,Cw,Iw,L1,ks1
    int sw = SC[w];

    auto v_px = concatVectors(w, uint2vc(sw), id, op);

    vector<unsigned char> L = gm_hmac(v_px.data(),v_px.size(),v_k1.data(),v_k1.size(),f_len); // F即sha256_hmac
    vector<unsigned char> Rw = gen_Rw(); //长度4
    vector<unsigned char> ks = gm_hmac(v_px.data(),v_px.size(),v_k2.data(),v_k2.size(),f_len); //长度8

    // 用id的vc形式替换Cw
    vector<unsigned char> Cw = id;

    // 判断上一次链尾的id_op_1是否为空,计算Iw
    vector<unsigned char> Iw;
    vector<unsigned char> HsR = gm_hmac(ks.data(),ks.size(),Rw.data(),Rw.size(),f_len*2); // H即sha256_hmac_v,长度16
    if (id_op_1.first == vector<unsigned char>{'\0'} && id_op_1.second == vector<unsigned char>{'\0'}){
        // 如果为空则用零字节向量zero_head求Iw
        vector<unsigned char> zero_head(f_len*2, 0);
        Iw = xor_encrypt_vc(HsR, zero_head);
    }
    else{
        // 如果不为空,则正常求L1,ks1,Iw
        vector<unsigned char> id_1=id_op_1.first;
        vector<unsigned char> op_1=id_op_1.second;

        auto v_px1 = concatVectors(w, uint2vc(sw), id_1, op_1);

        vector<unsigned char> L_1 = gm_hmac(v_px1.data(),v_px1.size(),v_k1.data(),v_k1.size(),f_len); //长度8
        vector<unsigned char> ks_1 = gm_hmac(v_px1.data(),v_px1.size(),v_k2.data(),v_k2.size(),f_len); //长度8
        vector<unsigned char> L_1_ks_1 = concatVectors(L_1, ks_1); //长度16
        Iw = xor_encrypt_vc(HsR, L_1_ks_1);
    }
    lastID[w]=make_pair(id,op);

    // com:client将Cwid=(L,Iw,Rw,Cw)发送给server,存入cipher_store
    Cwid.clear();
    Cwid = {L, Iw, Rw, Cw};
}

void FPKS2EClient::update_2(pair<vector<unsigned char>,vector<unsigned char>> w_id, vector<unsigned char> op, vector<vector<unsigned char>>& Cwid)
{
    vector<unsigned char> w=w_id.first;
    vector<unsigned char> id=w_id.second;

    vector<unsigned char> v_k1 = toByteVector(k1);
    vector<unsigned char> v_k2 = toByteVector(k2);

    // 1.client插入w到W
    this->W.insert(w);

    // 2.判断UC[w]是否为空,赋值uw,uw_1,sw
    int uw_1=0; // 默认为空表示未更新过
    if(UC.find(w)==UC.end()){
        SC[w]=0;
        UC[w]=0;
    }
    else{
        uw_1=UC[w]; // 求上一次链尾的uw_1
    }
    int sw = SC[w];
    UC[w]++;
    int uw = UC[w];

    // 3.计算中间量L,Rw,ks,Cw,Iw,L1,ks1
    auto v_px = concatVectors(w, uint2vc(sw), uint2vc(uw));

    vector<unsigned char> L = gm_hmac(v_px.data(),v_px.size(),v_k1.data(),v_k1.size(),f_len); // F即sha256_hmac
    vector<unsigned char> Rw = gen_Rw(); //长度4
    vector<unsigned char> ks = gm_hmac(v_px.data(),v_px.size(),v_k2.data(),v_k2.size(),f_len); //长度8

    // 用id的vc形式替换Cw
    vector<unsigned char> Cw = id;

    // 判断上一次链尾的uw_1是否为0,计算Iw,L1,ks1
    vector<unsigned char> Iw;
    vector<unsigned char> HsR = gm_hmac(ks.data(),ks.size(),Rw.data(),Rw.size(),f_len*2); // H即sha256_hmac_v,长度16
    if (uw_1 == 0){
        // 如果为空则用零字节向量zero_head求Iw
        vector<unsigned char> zero_head(f_len*2, 0);
        Iw = xor_encrypt_vc(HsR, zero_head);
    }
    else{
        // 如果不为空,则正常求L1,ks1,Iw
        vector<unsigned char> v_px1 = concatVectors(w, uint2vc(sw), uint2vc(uw_1));

        vector<unsigned char> L_1 = gm_hmac(v_px1.data(),v_px1.size(),v_k1.data(),v_k1.size(),f_len); //长度8
        vector<unsigned char> ks_1 = gm_hmac(v_px1.data(),v_px1.size(),v_k2.data(),v_k2.size(),f_len); //长度8
        vector<unsigned char> L_1_ks_1 = concatVectors(L_1, ks_1); //长度16
        Iw = xor_encrypt_vc(HsR, L_1_ks_1);
    }

    Cwid = {L, Iw, Rw, Cw};
    // cipher_store[L] = Cwid;
}

int FPKS2EClient::trapdoor1(vector<unsigned char> w, vector<unsigned char>& L, vector<unsigned char>& ks)
{
    vector<unsigned char> v_k1 = toByteVector(k1);
    vector<unsigned char> v_k2 = toByteVector(k2);

    L.clear();
    ks.clear();

    if (SC.find(w) == SC.end()) {
        string wstring(w.begin(),w.end());
        cout<<"trapdoor gen failed,SC["+wstring+"] is empty!";
        return 0;
    }

    int sw = SC[w]; //当前关键字的查询次数,查询之后查询次数+1
    SC[w]++;
    pair<vector<unsigned char>, vector<unsigned char>> id_op = lastID[w];

    // lastID[w] = pair<unsigned int, char>{0, '\0'}; // set lastID[w] null

    vector<unsigned char> id = id_op.first;
    vector<unsigned char> op = id_op.second;

    auto v_px = concatVectors(w, uint2vc(sw), id ,op);

    L = gm_hmac(v_px.data(),v_px.size(),v_k1.data(),v_k1.size(),f_len); //长度8
    ks = gm_hmac(v_px.data(),v_px.size(),v_k2.data(),v_k2.size(),f_len); //长度8
    
    return 0;
}

int FPKS2EClient::trapdoor2(vector<unsigned char> w, vector<unsigned char>& L, vector<unsigned char>& ks)
{
    vector<unsigned char> v_k1 = toByteVector(k1);
    vector<unsigned char> v_k2 = toByteVector(k2);

    L.clear();
    ks.clear();

    if (SC.find(w) == SC.end()) {
        string wstring(w.begin(),w.end());
        cout<<"trapdoor gen failed,SC["+wstring+"] is empty!";
        return 0;
    }

    // 求sw，uw，更新SC[w]
    int sw = SC[w]; //当前关键字的查询次数,查询之后查询次数+1
    SC[w]++;
    int uw = UC[w];

    auto v_px = concatVectors(w, uint2vc(sw), uint2vc(uw));  // px = w||sw||uw

    // get L ks
    L = gm_hmac(v_px.data(),v_px.size(),v_k1.data(),v_k1.size(),f_len); //长度8
    ks = gm_hmac(v_px.data(),v_px.size(),v_k2.data(),v_k2.size(),f_len); //长度8

    return 0;
}

void FPKS2EClient::search_output(map<vector<unsigned char>,vector<vector<unsigned char>>> search_result, const std::string& search_output_dir) {
    string file_path = search_output_dir;
    // 打开文件
    std::ofstream outfile(file_path);

    if (!outfile.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + file_path);
    }

    for (const auto& entry : search_result) {
        vector<unsigned char> v_w = entry.first;
        string w(v_w.begin(), v_w.end());
        auto ids = entry.second;
        // 写入 w 和 id 信息
        outfile << "w: " << w << std::endl;
        outfile << "id:";
        if(ids.size()==0){
            outfile << " NULL" << endl;
            continue;
        }
        for (auto v_id : ids) {
            unsigned int id = vc2uint(v_id);
            outfile << " " << id; // 每个 id 之间空格
        }
        outfile << std::endl;
    }

    // 关闭文件
    outfile.close();

    cout<<"查询结果已写入"<<search_output_dir<<endl;
}

vector<vector<unsigned char>> FPKS2EClient::gen_set_query(set<vector<unsigned char>> &W, size_t size_query){
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

vector<unsigned char> FPKS2EClient::gen_Rw() {
    // 初始化随机数生成器
    std::srand(std::time(0));  // 用当前时间作为种子

    // 创建长度为4的vector
    std::vector<unsigned char> Rw(f_len);

    // 填充vector，生成0到255之间的随机数
    for (int i = 0; i < f_len; ++i) {
        Rw[i] = std::rand() % 256;  // 生成0到255之间的随机数
    }

    return Rw;
}

vector<vector<unsigned char>> FPKS2EClient::dec_result(vector<vector<unsigned char>> S)
{
    vector<vector<unsigned char>> getR;
    for(auto &C:S){
        getR.push_back(C);
    }
    return getR;
}

void FPKS2EClient::batch_dec(map<vector<unsigned char>, vector<vector<unsigned char>>> Ss)
{
    this->search_result.clear();

    for (const auto& entry : Ss) {
        vector<unsigned char> w = entry.first;         // 获取关键字
        vector<vector<unsigned char>> S = entry.second;       // 获取对应的id集合
        vector<vector<unsigned char>> ids = dec_result(S);
        this->search_result[w] = ids;
    }
}

void FPKS2EServer::setup()
{
    std::cout << "Setting up FPKS2E Server..." << std::endl;
}

void FPKS2EServer::init_key(const char key1[keylen], const char key2[keylen])
{
    memcpy(k1, key1, keylen);
    memcpy(k2, key2, keylen);
}

void FPKS2EServer::reset_cipher_store()
{
    cipher_store.clear();
}

void FPKS2EServer::re_update(vector<vector<unsigned char>> &Cwid)
{
    vector<unsigned char> L = Cwid[0]; //长度8
    vector<unsigned char> Iw = Cwid[1]; //长度8
    vector<unsigned char> Rw = Cwid[2]; //长度4
    vector<unsigned char> Cw = Cwid[3]; //长度4

    this->cipher_store[L] = {Iw, Rw, Cw};
}

// search 方法
vector<vector<unsigned char>> FPKS2EServer::search(vector<unsigned char> L, vector<unsigned char> ks) 
{
    vector<unsigned char> s_L = L; //长度4
    vector<unsigned char> s_ks = ks; //长度4
    vector<vector<unsigned char>> S; //存Cw
    vector<unsigned char> zero_L(f_len, 0); //表示空,循环结束标志
    while(s_L!=zero_L){
        vector<unsigned char> Iw=cipher_store[s_L][0]; //长度8
        vector<unsigned char> Rw=cipher_store[s_L][1];
        vector<unsigned char> Cw=cipher_store[s_L][2]; //为id
        vector<unsigned char> HsR = gm_hmac(s_ks.data(),s_ks.size(),Rw.data(),Rw.size(),f_len*2);
        vector<unsigned char> L_1_ks_1 = xor_decrypt_vc(HsR, Iw); //长度8
        vector<unsigned char> L_1; //长度4
        vector<unsigned char> ks_1; //长度4
        get_L_1_ks_1(L_1_ks_1, L_1, ks_1, f_len);
        S.push_back(Cw);
        // update s_L s_ks
        s_L=L_1;
        s_ks=ks_1;
    }

    return S;
}

void FPKS2EServer::copy_cipher_store(map<vector<unsigned char>, vector<vector<unsigned char>>> store_in)
{
    this->cipher_store.clear();

    this->cipher_store=store_in;
}




