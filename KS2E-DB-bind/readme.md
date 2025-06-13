# KS2E imp（Branch）DB版

## Version

v2 on 6.6

by Zhenpeng Zhou

## Requirement

* ubuntu 20.04
* g++ 9.4.0 
* openGauss 6.0.1 (LTS)
* PostgreSQL 12.22（libpq）
* GmSSL 3.1.2
* OpenSSL 1.1.1w

## **Description**

- 通过c++实现的KS2E，具备更新，搜索，分享以及重建功能
- 实现了client类和server类，在一个程序中通过中间参数隔离模拟通信
- 通过GMssl实现hash，hmac
- 所有输入输出数据以 `vector<unsigned char>`类型传递,如keyword,id,op
- openGauss中的密文索引，密文文件数据以bytea类型存储，通过二进制格式读写

## Run

* 在当前目录下make run
* 自行准备模拟明文，本代码以读取数据库形式导入
