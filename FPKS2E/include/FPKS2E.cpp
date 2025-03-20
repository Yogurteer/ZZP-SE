#include "FPKS2E.h"
#include <iostream>

// 构造函数
FPKS2E::FPKS2E() {
}

// 析构函数
FPKS2E::~FPKS2E() {
}

// setup 方法
void FPKS2E::setup() {
    generate_random_key(k1,32);
    generate_random_key(k2,32);
    std::cout << "Setting up FPKS2E system..." << std::endl;
}

void FPKS2E::update(pair<string,unsigned int> w_id, char op){
    string w=w_id.first;
    unsigned int id=w_id.second;

    vector<unsigned char> v_k1 = toByteVector(k1);
    vector<unsigned char> v_k2 = toByteVector(k2);

    // 1.client插入w到W
    W.insert(w);

    // 2.判断lastID[w]是否为空,赋值id_op
    pair<unsigned int,char> id_op_1; //默认为空
    if(lastID.find(w)==lastID.end()){
        SC[w]=0;
    }
    else{
        id_op_1=lastID[w]; // 求上一次链尾的id_op_1
    }

    // 3.计算中间量L,Rw,ks,Cw,Iw,L1,ks1
    int sw = SC[w];
    string px_str = w + to_string(sw) + to_string(id) + to_string(op);  // px = w||sw||id||op
    const char* px = px_str.c_str();
    vector<unsigned char> v_px = toByteVector(px);

    vector<unsigned char> L = gm_hmac(v_px.data(),v_px.size(),v_k1.data(),v_k1.size(),f_len); // F即sha256_hmac
    vector<unsigned char> Rw = gen_Rw(); //长度4
    vector<unsigned char> ks = gm_hmac(v_px.data(),v_px.size(),v_k2.data(),v_k2.size(),f_len); //长度8

    // 用id的vc形式替换Cw
    vector<unsigned char> v_id = uintToBytes(id);
    vector<unsigned char> Cw = v_id;

    // 判断上一次链尾的id_op_1是否为空,计算Iw
    vector<unsigned char> Iw;
    vector<unsigned char> HsR = gm_hmac(ks.data(),ks.size(),Rw.data(),Rw.size(),f_len*2); // H即sha256_hmac_v,长度16
    if (id_op_1.first == 0 && id_op_1.second == '\0'){
        // 如果为空则用零字节向量zero_head求Iw
        vector<unsigned char> zero_head(f_len*2, 0);
        Iw = xor_encrypt_vc(HsR, zero_head);
    }
    else{
        // 如果不为空,则正常求L1,ks1,Iw
        unsigned int id_1=id_op_1.first;
        char op_1=id_op_1.second;
        string px1_str = w + to_string(sw) + to_string(id_1) + to_string(op_1);  // px1 = w||sw||id_1||op_1
        const char* px1 = px1_str.c_str();
        vector<unsigned char> v_px1 = toByteVector(px1);

        vector<unsigned char> L_1 = gm_hmac(v_px1.data(),v_px1.size(),v_k1.data(),v_k1.size(),f_len); //长度8
        vector<unsigned char> ks_1 = gm_hmac(v_px1.data(),v_px1.size(),v_k2.data(),v_k2.size(),f_len); //长度8
        vector<unsigned char> L_1_ks_1 = concatenate_vectors(L_1, ks_1); //长度16
        Iw = xor_encrypt_vc(HsR, L_1_ks_1);
    }
    lastID[w]=make_pair(id,op);

    // com:client将Cwid=(L,Iw,Rw,Cw)发送给server,存入EDB
    vector<vector<unsigned char>> Cwid = {Iw, Rw, Cw};
    EDB[L] = Cwid;
}

void FPKS2E::batchupdate(char *update_dir, char op)
{
    std::cout << "Updating FPKS2E data..." << std::endl;
    // 从文件读入DB
    map<string, vector<unsigned int>> Ulist;
    Ulist.clear(); // 清空Ulist
    cout << "Update Dataset source dir: " << update_dir << endl;
    read_keywords(update_dir, Ulist);

    // 遍历Ulist中的每个关键字
    for (const auto& entry : Ulist) {
        string w = entry.first;         // 获取关键字
        vector<unsigned int> ids = entry.second;       // 获取对应的id集合
        for(auto &id:ids){
            pair w_id = make_pair(w, id);
            update(w_id, op);
        }
    }


}

// search 方法
vector<unsigned int> FPKS2E::search(string w) {

    // client

    vector<unsigned char> v_k1 = toByteVector(k1);
    vector<unsigned char> v_k2 = toByteVector(k2);
    // 判断SC[w]是否为空,空则之间返回空结果
    vector<unsigned int> getRw; // 存储所有查询的id集合
    if (SC.find(w) == SC.end()) {
        return getRw;
    }
    // 对于SC[w]非空,计算L和ks
    vector<vector<unsigned char>> Tw; //存L和ks
    int sw = SC[w]; //当前关键字的查询次数,查询之后查询次数+1
    SC[w]++;
    pair<unsigned int,char> id_op = lastID[w];
    // set lastID[w] null
    lastID[w] = pair<unsigned int, char>{0, '\0'};
    unsigned id = id_op.first;
    char op = id_op.second;
    string px_str = w + to_string(sw) + to_string(id) + to_string(op);  // px = w||sw||id||op
    const char* px = px_str.c_str();
    vector<unsigned char> v_px = toByteVector(px);

    vector<unsigned char> L = gm_hmac(v_px.data(),v_px.size(),v_k1.data(),v_k1.size(),f_len); //长度8
    vector<unsigned char> ks = gm_hmac(v_px.data(),v_px.size(),v_k2.data(),v_k2.size(),f_len); //长度8
    Tw.insert(Tw.end(),{L,ks});

    // com:client将Tw发送给server

    string Token_dir1 = "dataset/com/c2s_Tw.txt";
    write_Token(Tw, Token_dir1);
    vector<vector<unsigned char>> s_Tw = read_Token(Token_dir1);

    // server

    // 解析Tw得到L和ks,初始化查询结果S
    // 遍历EDB链,取出所有Cw存入S
    vector<unsigned char> s_L = L; //长度4
    vector<unsigned char> s_ks = ks; //长度4
    vector<vector<unsigned char>> S; //存Cw
    vector<unsigned char> zero_L(f_len, 0); //表示空,循环结束标志
    while(s_L!=zero_L){
        vector<unsigned char> Iw=EDB[s_L][0]; //长度8
        vector<unsigned char> Rw=EDB[s_L][1];
        vector<unsigned char> Cw=EDB[s_L][2]; //为id
        vector<unsigned char> HsR = gm_hmac(s_ks.data(),s_ks.size(),Rw.data(),Rw.size(),f_len*2);
        vector<unsigned char> L_1_ks_1 = xor_decrypt_vc(HsR, Iw); //长度8
        vector<unsigned char> L_1; //长度4
        vector<unsigned char> ks_1; //长度4
        get_L_1_ks_1(L_1_ks_1, L_1, ks_1, f_len);
        S.push_back(Cw);
        EDB.erase(s_L); // 删除访问过的EDB条目
        // update s_L s_ks
        s_L=L_1;
        s_ks=ks_1;
    }
    
    // com:server将S发送给client

    string Token_dir2 = "dataset/com/s2c_S.txt";
    write_Token(S, Token_dir2);
    vector<vector<unsigned char>> c_S = read_Token(Token_dir2);

    // client

    // 初始化原始结果R
    vector<pair<unsigned int, char>> R;
    // 从S中解密得到id||op存入R,合并得到Rw
    for(auto &C:S){
        unsigned int get_id = bytesToUint(C);
        getRw.push_back(get_id);
    }
    // 函数返回Rw表示最终查询的id集合
    return getRw;
}

map<string,vector<unsigned int>> FPKS2E::batchsearch(vector<string> Wq)
{
    std::cout << "Performing FPKS2E search..." << std::endl;
    search_result.clear(); // 清空search_result
    for(auto &w:Wq){
        vector<unsigned int> Rw = search(w);
        search_result[w]=Rw;
    }
    return search_result;
}

void FPKS2E::search_output(map<string,vector<unsigned int>> search_result, const std::string& search_output_dir) {
    string file_path = search_output_dir;
    // 打开文件
    std::ofstream outfile(file_path);

    if (!outfile.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + file_path);
    }

    for (const auto& entry : search_result) {
        const std::string& w = entry.first;
        const std::vector<unsigned int>& ids = entry.second;
        // 写入 w 和 id 信息
        outfile << "w: " << w << std::endl;
        outfile << "id:";
        if(ids.size()==0){
            outfile << " NULL" << endl;
            continue;
        }
        for (unsigned int id : ids) {
            outfile << " " << id; // 每个 id 之间空格
        }
        outfile << std::endl;
    }

    // 关闭文件
    outfile.close();

    cout<<"查询结果已写入"<<search_output_dir<<endl;
}

vector<string> FPKS2E::gen_set_query(set<string> &W, size_t size_query) {
    vector<string> set_query;

    // 1. 将 set<string> 转换为 vector<string>，方便进行随机选择
    std::vector<std::string> temp(W.begin(), W.end());

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

vector<unsigned char> FPKS2E::gen_Rw() {
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
