# DBOG

通用的访问openGauss数据库的头文件，

## Version

v1 on 6.5 

by Zhenpeng Zhou

## **Description**

- 用于访问openGauss数据库，具有连接，创建表，插入条目，查询条目，修改条目，删除条目，清空表，删除表功能
- 同时支持二进制格式或文本格式的操作

## 类DBOGaccess

### 主要成员变量

- conninfo：连接信息
- conn：数据库连接变量
- res：存储SQL执行返回值

### 主要成员函数

**exit_nicely**

````c++
static void exit_nicely(PGconn* conn)
````

* `function`：断开连接并终止程序
* `conn`：数据库连接变量

**DBOGaccess**

````c++
DBOGaccess()
DBOGaccess(const char* info) :conninfo(info)
````

* `function`：类构造函数，连接数据库，在此设置连接信息
* `info`:设置数据连接信息
  * 参考:"dbname=postgres port=5432 host=222.20.126.206 user=admin password=hust@123"

**exceSQL**

````c++
PGresult* exceSQL(vector<vector<uint8_t>>& params, const char* command, int paramformat = 1, int resultformat = 1)
````

* `function`：带参数执行SQL语句
* `params `：参数,类型二维字节向量
* `command`：SQL语句
* `paramformat`:参数传输格式,0文本,1二进制,默认1
* `resultformat`:返回结果格式,0文本,1二进制,默认1

**createTable**

```c++
void createTable(const char* command)
```

* `function`：创建表
* `command`：SQL语句

**writeData**

```c++
void writeData(vector<vector<uint8_t>>& datavecs, const char* command, int paramformat = 1)
```

* `function`：写入一条记录，默认以二进制写
* `datavecs`：待写入数据，二维字节向量，每个字节向量表示一个元素数据
* `command`：SQL语句
* `paramformat`:参数传输格式,0文本,1二进制,默认1

**readData**

```c++
vector<vector<vector<uint8_t>>> readData(PGresult*& res, vector<vector<uint8_t>>& query_data, const char* command, int paramformat = 1, int resultformat = 1)
```

* `function`：读取一条记录，返回结果为三维字节向量,第一维表示一行记录,第二维表示一个元素,vector<uint8_t>是描述一个元素的基本单位
* `res`:存储SQL执行的返回值,可用于后续自定义访问
* `query_data`：用于匹配的查询元素，结合实际的SQL语句
* `command`：SQL语句
* `paramformat`:参数传输格式,0文本,1二进制,默认1
* `resultformat`:返回结果格式,0文本,1二进制,默认1

**alterData**

```c++
void alterData(vector<vector<uint8_t>>& datavecs, const char* command, int paramformat = 1)
```

* `function`：修改一条记录
* `datavecs`：数据参数，结合实际的SQL语句
* `command`：SQL语句
* `paramformat`:参数传输格式,0文本,1二进制,默认1

**deleteData**

```c++
void deleteData(vector<vector<uint8_t>>& datavecs, const char* command, int paramformat = 1)
```

* `function`：删除一条记录
* `datavecs`：数据参数，结合实际的SQL语句
* `command`：SQL语句

**clearTable**

```c++
void clearTable(const char* command)
```

* `function`：清空表
* `command`：SQL语句

**dropTable**

```c++
void dropTable(const char* command)
```

* `function`：删除表
* `command`：SQL语句



