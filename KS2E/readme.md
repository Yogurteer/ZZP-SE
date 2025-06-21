# KS2E imp（Branch）单机版

## Version

v1 on 4.3

by Zhenpeng Zhou

## Env

* Ubuntu 22.04
* g++ 9.4.0 
* GmSSL 3.1.2 
* OpenSSL 3.0.2

## **Description**

- 实现双链隐式倒排索引结构
- 关键字可搜索对称加密框架
- 通过GMssl实现hash，hmac，安全信道
- 所有输入输出数据以 `vector<unsigned char>`类型传递,如keyword,id,op

## Run

* 在当前目录下make run

## Goal

* [X] 在FPKS2E基础上，从单链拓展到双链
* [X] lastID存id而不是id_op
* [X] 不需要UC，SC计数器
* [X] unsigned char*作为二进制数据处理而不是字符串
* [X] 完善sharetoken，share功能，拓展user，owner三种用户
* [X] 与W空间类似,记录id空间,W||id,从而实现按id搜索功能排除重复更新
* [X] 当owner分享给user一个id对应的{w}时,user除了更新每个w下的lastID,是否需要更新id下的最后一个w?是
