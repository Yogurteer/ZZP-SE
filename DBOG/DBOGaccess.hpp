#pragma once

#include <libpq-fe.h>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

inline std::string toHexString(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    oss << std::hex;
    for (uint8_t byte : data) {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

inline std::vector<uint8_t> fromHexString(const std::string& hexStr) {
    std::vector<uint8_t> bytes;
    // 处理奇数长度情况（理论上不会出现）
    size_t len = hexStr.length();
    if (len % 2 != 0) {
        fprintf(stderr, "Invalid hex string length: %zu\n", len);
        return bytes;
    }

    // 每两个字符转换一个字节
    for (size_t i = 0; i < len; i += 2) {
        std::string byteStr = hexStr.substr(i, 2);
        char* end;
        long byte = strtol(byteStr.c_str(), &end, 16);

        // 检查转换有效性
        if (*end != '\0') {
            fprintf(stderr, "Invalid hex character: %s\n", byteStr.c_str());
            bytes.clear();
            return bytes;
        }
        bytes.push_back(static_cast<uint8_t>(byte));
    }
    return bytes;
}

class DBOGaccess {
public:
    const char* conninfo;
    PGconn* conn;
    PGresult* res;
    static void exit_nicely(PGconn* conn)
    {
        PQfinish(conn);
        exit(1);
    }


    DBOGaccess() {
        conninfo = "dbname=postgres port=5432 host=222.20.126.206 user=admin password=hust@123";
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK)
        {
            fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
            exit_nicely(conn);
        }
        printf("数据库连接成功\n");
    }
    DBOGaccess(const char* info) :conninfo(info) {
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK)
        {
            fprintf(stderr, "Connection to database failed: %s",PQerrorMessage(conn));
            exit_nicely(conn);
        }
    }
    ~DBOGaccess() {
        PQfinish(conn);
    }

    PGresult* exceSQL(vector<vector<uint8_t>>& params, const char* command, int paramformat = 1, int resultformat = 1) {
        // std::cout << "SQL Command:\n" << command << std::endl;
        PGresult* res;
        if(params.empty()) {
            // res = PQexec(conn, command);
            res = PQexecParams(conn, command, 0, NULL, NULL, NULL, NULL, resultformat);
        }
        else{
            // 获取字节向量的数量
            size_t n = params.size();
            // 动态构建 paramValues, paramLengths, paramFormats 数组
            const char* paramValues[n];
            int paramLengths[n];
            int paramFormats[n];
            // 初始化 paramFormats 数组，将所有参数都设置为二进制格式
            std::fill(paramFormats, paramFormats + n, paramformat); // 所有参数都设置为二进制格式
            // 遍历 params，为每个字节向量设置对应的参数
            for (size_t i = 0; i < n; ++i) {
                paramValues[i] = (const char*)params[i].data(); // 获取字节向量的数据指针
                paramLengths[i] = params[i].size();            // 获取字节向量的大小（长度）
            }
            // 执行 SQL 命令，插入一行记录
            res = PQexecParams(conn, command, n, NULL, paramValues, paramLengths, paramFormats, resultformat);
        }

        // 检查执行结果
        if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
            cout<<PQresultStatus(res)<<endl;
            std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            exit_nicely(conn);
        }

        // cout<<"exceed SQL successfully."<<endl;

        return res;
    }

    // eg: "CREATE TABLE your_table (vol1 BYTEA, vol2 BYTEA, vol3 BYTEA);"
    void createTable(const char* command) {
        vector<vector<uint8_t>> params; // 空的二维字节向量
        PGresult* res = exceSQL(params, command);
        cout << "Table created successfully." << endl;
        // 清理结果
        PQclear(res);
    }

    // eg: "INSERT INTO your_table (vol1, vol2, vol3) VALUES ($1, $2, $3) ON CONFLICT (vol1) DO NOTHING;"
    void writeData(vector<vector<uint8_t>>& datavecs, const char* command, int paramformat = 1) {
        PGresult* res = exceSQL(datavecs, command);
        cout << "Data inserted successfully." << endl;
        // 清理结果
        PQclear(res);
    }

    // eg: "SELECT vol1,vol2,vol3 FROM your_table WHERE v1 = $1;"
    vector<vector<vector<uint8_t>>> readData(PGresult*& res, vector<vector<uint8_t>>& query_data, const char* command, int paramformat = 1, int resultformat = 1) {
        vector<vector<vector<uint8_t>>> query_outputs; // 用于存储查询结果的三维字节向量
        vector<vector<uint8_t>> row_data; // 用于存储每一行的字节向量

        res = exceSQL(query_data, command, resultformat);

        // 检查返回的行数是否为 1
        int rowCount = PQntuples(res);
        if (rowCount == 0) {
            fprintf(stderr, "Failed!Query get 0 rows");
            PQclear(res);
            exit_nicely(conn);
        }

        cout << "Query returned " << rowCount << " row(s)." << endl;
        // 获取第一行的每一列数据
        int rows = PQntuples(res);
        int cols = PQnfields(res);
        for (int row = 0; row < rows; ++row) {
            // 清空当前行的数据
            row_data.clear();
            // 遍历每一列
            for (int col = 0; col < cols; ++col) {
                // 获取当前列的值
                char* value = PQgetvalue(res, row, col);
                // 将列值转存为 std::vector<uint8_t>
                std::vector<uint8_t> result(value, value + PQgetlength(res, row, col));
                // 将当前列的字节向量添加到当前行数据中
                row_data.push_back(result);
            }
            // 将当前行数据添加到结果中
            query_outputs.push_back(row_data);
        }
        // 返回最终的三维字节向量
        return query_outputs;
    }

    // eg: "UPDATE your_table SET vol2 = $1 WHERE vol1 = $2;"
    void alterData(vector<vector<uint8_t>>& datavecs, const char* command, int paramformat = 1){
        PGresult* res = exceSQL(datavecs, command);
        cout << "Data updated successfully." << endl;
        // 清理结果
        PQclear(res);
    }

    // eg: "DELETE FROM your_table WHERE vol1 = $1;"
    void deleteData(vector<vector<uint8_t>>& datavecs, const char* command, int paramformat = 1){
        PGresult* res = exceSQL(datavecs, command);
        cout << "Data deleted successfully." << endl;
        // 清理结果
        PQclear(res);
    }

    // eg: "TRUNCATE TABLE your_table;"
    void clearTable(const char* command){
        vector<vector<uint8_t>> params; // 空的二维字节向量
        PGresult* res = exceSQL(params, command);
        cout<< "Table cleared successfully." << endl;
        // 清理结果
        PQclear(res);
    }

    // eg: "DROP TABLE your_table;"
    void dropTable(const char* command) {
        vector<vector<uint8_t>> params; // 空的二维字节向量
        PGresult* res = exceSQL(params, command);
        cout<< "Table dropped successfully." << endl;
        // 清理结果
        PQclear(res);
    }
};
