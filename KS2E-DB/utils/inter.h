#pragma once

#include "KS2E.h"
#include "utils.h"

using namespace std;

const string share_id_dir = "dataset/view/u_share_id_result.txt";
const string share_w_dir = "dataset/view/u_share_w_result.txt";

void fullupdate(KS2EOwner& o, KS2EServer& s, 
    pair<vector<unsigned char>, vector<unsigned char>> w_id, vector<unsigned char> op);

void batchupdate(KS2EOwner &o, KS2EServer &s, char *update_dir, vector<unsigned char> op);

void fullshare_id(KS2EUser &u, KS2EServer &s, vector<vector<unsigned char>> &Pid);

void fullshare_w(KS2EUser &u, KS2EServer &s, vector<vector<unsigned char>> &Pw);

void fullsearch_id(string client_type, KS2EClient &c, KS2EServer &s, vector<unsigned char> id, vector<vector<unsigned char>>& getws);

void batchsearch_id(string client_type, KS2EClient &c, KS2EServer &s, vector<vector<unsigned char>> IDq, 
                    map<vector<unsigned char>, vector<vector<unsigned char>>> &result);

void batchsearch_w(string client_type, KS2EClient &c, KS2EServer &s, vector<vector<unsigned char>> Wq, 
    map<vector<unsigned char>, vector<vector<unsigned char>>> &result);