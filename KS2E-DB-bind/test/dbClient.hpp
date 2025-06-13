#ifndef DBCLIENT_HPP
#define DBCLIENT_HPP


#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <libpq-fe.h>
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

class BestieCipher
{
public:
    unsigned char D[32];
    unsigned char IV[16];
    unsigned char C[64];
    // 将 D/IV/C 合并为连续的 unsigned char*
    unsigned char* serialize() const{
        unsigned char* buffer = new unsigned char[totalSize()];
        memcpy(buffer, D, sizeof(D));
        memcpy(buffer + sizeof(D), IV, sizeof(IV));
        memcpy(buffer + sizeof(D) + sizeof(IV), C, sizeof(C));
        return buffer;
    }

    // 从 buffer 中还原 D/IV/C
    void deserialize(const unsigned char* buffer) {
        memcpy(D, buffer, sizeof(D));
        memcpy(IV, buffer + sizeof(D), sizeof(IV));
        memcpy(C, buffer + sizeof(D) + sizeof(IV), sizeof(C));
    }

    // 计算总字节数
    static constexpr size_t totalSize() {
        return sizeof(D) + sizeof(IV) + sizeof(C); // 32 + 16 + 64 = 112
    }
};

class dbClient {

private:
    const char* conninfo;
    PGconn* conn;
    PGresult* res;
    static void exit_nicely(PGconn* conn)
    {
        PQfinish(conn);
        exit(1);
    }

public:
    dbClient() {
        conninfo = "dbname=bestie port=5432 host='localhost' application_name=test connect_timeout=5 sslmode=allow user='test' password='Test2002'";
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK)
        {
            fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
            exit_nicely(conn);
        }
    }
    dbClient(const char* info) :conninfo(info) {
        conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK)
        {
            fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
            exit_nicely(conn);
        }
    }
    ~dbClient() {
        PQfinish(conn);
    }
    void writeData(std::map<long long, std::vector<uint8_t>>& rows, std::map<long long, std::vector<uint8_t>>& file_keys) {
        const char* command = "INSERT INTO encrypted_file_table (file_id,encrypted_file,kf) VALUES ($1, $2, $3);";
        for (auto item : rows) {
            const char* paramValues[3];
            const std::string idstr = std::to_string(item.first);
            paramValues[0] = idstr.c_str();
            paramValues[1] = (const char*)item.second.data();
            paramValues[2] = (const char*)file_keys[item.first].data();
            int paramLengths[3] = { idstr.size() ,item.second.size(),file_keys[item.first].size()};
            const int paramFormats[3] = { 0,1,1 };
            res = PQexecParams(conn, command, 3, NULL, paramValues, paramLengths, paramFormats, 1);
            if (PQresultStatus(res) != PGRES_COMMAND_OK)
            {
                fprintf(stderr, "UPDATE command failed: %s", PQerrorMessage(conn));
                PQclear(res);
                exit_nicely(conn);
            }
        }
    }
    // 从数据库读取二进制数据
    std::vector<uint8_t> readData(long long key,  std::vector<uint8_t>& file_key) {
        const char* command = "SELECT encrypted_file,kf FROM encrypted_file_table WHERE file_id = $1 ";
        const char* paramValues[1];
        const std::string idstr = std::to_string(key);
        paramValues[0] = idstr.c_str();
        int paramLengths[1] = { 0 };
        paramLengths[0] = idstr.size();
        const int paramFormats[1] = { 0 };
        // 执行查询
        res = PQexecParams(conn, command, 1, NULL, paramValues, paramLengths, paramFormats, 1);
        // 检查查询状态
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            return std::vector<uint8_t>(0);
        }
        // 检查结果行数
        int rowCount = PQntuples(res);
        if (rowCount == 0) {
            fprintf(stderr, "No data found for id: %lld\n", key);
            PQclear(res);
            return std::vector<uint8_t>(0);
        }
        char* Value = PQgetvalue(res, 0, 0);
        std::vector<uint8_t> result(Value, Value + PQgetlength(res,0,0));
        //std::cout << "result size:" << result.size() << std::endl;
        char* Value2 = PQgetvalue(res, 0, 1); 
        file_key.assign(reinterpret_cast<const uint8_t*>(Value2),
            reinterpret_cast<const uint8_t*>(Value2 + strlen(Value2)));
        PQclear(res);
        return result;
    }
    void write_count(const std::string& keyword, int cnt_upd, int cnt_srch) {
        // 转义字符串防止SQL注入
        char* escaped_key = PQescapeLiteral(conn, keyword.c_str(), keyword.size());

        // 未存在时插入，存在时更新
        std::string sql =
            "INSERT INTO count (keyword, cnt_upd, cnt_srch) "
            "VALUES (" + std::string(escaped_key) + ", "
            + std::to_string(cnt_upd) + ", "
            + std::to_string(cnt_srch) + ") "
            "ON DUPLICATE KEY update cnt_upd=values(cnt_upd),cnt_srch=values(cnt_srch);";

        PQfreemem(escaped_key);  // 释放转义内存

        // 执行语句
        res = PQexec(conn, sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "UPSERT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            exit_nicely(conn);
        }
        PQclear(res);
    }
    // 从count表读取计数对
    std::pair<int, int> read_count(const std::string& keyword){
        // 转义关键字
        char* escaped_key = PQescapeLiteral(conn, keyword.c_str(), keyword.size());

        std::string sql =
            "SELECT cnt_upd, cnt_srch FROM count "
            "WHERE keyword = " + std::string(escaped_key) + ";";

        PQfreemem(escaped_key);

        // 执行查询
        res = PQexec(conn, sql.c_str());
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            return { -1, -1 };  // 错误码
        }

        // 处理结果
        int row_count = PQntuples(res);
        if (row_count == 0) {
            //write_count(keyword, 0, 0);
            return { 0, 0 };  // 默认值
        }

        // 解析整数值
        int upd = atoi(PQgetvalue(res, 0, 0));  // 将查询结果第1列转换为整数
        int srch = atoi(PQgetvalue(res, 0, 1)); // 将查询结果第2列转换为整数
        PQclear(res);

        return { upd, srch };
    }
    void clientClear() {
        std::string sql = "TRUNCATE TABLE count,encrypted_file_table;";
        res = PQexec(conn, sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::string err_msg = PQerrorMessage(conn);
            PQclear(res);
            throw std::runtime_error("清空表失败: " + err_msg);
        }

        PQclear(res);
    }
    void write_index(const std::string& label,const BestieCipher& cipher) {
        if (PQstatus(conn) == CONNECTION_BAD) {
            fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
            PQfinish(conn);
            return;
        }
        const char* command = "INSERT INTO encrypted_index_table (label,index) VALUES ($1, $2) ON DUPLICATE KEY update index=values(index);";
        const char* paramValues[2];
        paramValues[0] = label.c_str();
        paramValues[1] = (const char*)cipher.serialize();
        int paramLengths[2]= { label.size(),32+16+64 };
        const int paramFormats[2] = { 1,1 };
        res = PQexecParams(conn, command, 2, NULL, paramValues, paramLengths, paramFormats, 1);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "插入数据失败: %s\n", PQerrorMessage(conn));
        }
    }
    BestieCipher read_index(const std::string& label) {
        BestieCipher cipher = {};
        if (PQstatus(conn) == CONNECTION_BAD) {
            fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
            PQfinish(conn);
            return cipher;
        }
        const char* command = "SELECT index FROM encrypted_index_table WHERE label = $1";
        const char* paramValues[1];
        paramValues[0] = label.c_str();
        int paramLengths[1] = { 0 };
        paramLengths[0] = label.size();
        const int paramFormats[1] = { 1 };
        res = PQexecParams(conn, command, 1, NULL, paramValues, paramLengths, paramFormats, 1);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
            PQclear(res);
            return cipher;
        }

        // 检查结果行数
        int rowCount = PQntuples(res);
        if (rowCount == 0) {
            fprintf(stderr, "No data found for label: %s\n",label.c_str());
            PQclear(res);
            return cipher;
        }
        char* Value = PQgetvalue(res, 0, 0);
        cipher.deserialize((const unsigned char*)Value);
        return cipher;
    }
    void erase_index(const std::string& label) {
        if (PQstatus(conn) == CONNECTION_BAD) {
            fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
            PQfinish(conn);
            return;
        }
        const char* command = "DELETE FROM encrypted_index_table WHERE label = $1;";
        const char* paramValues[1];
        paramValues[0] = label.c_str();
        int paramLengths[1] = { 0 };
        paramLengths[0] = label.size();
        const int paramFormats[1] = { 1 };
        res = PQexecParams(conn, command, 1, NULL, paramValues, paramLengths, paramFormats, 1);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "delete fail: %s\n", PQerrorMessage(conn));
        }
    }
    void serverClear() {
        std::string sql = "TRUNCATE TABLE encrypted_index_table;";
        res = PQexec(conn, sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::string err_msg = PQerrorMessage(conn);
            PQclear(res);
            throw std::runtime_error("清空表失败: " + err_msg);
        }

        PQclear(res);
    }
};

#endif // !DBCLIENT_HPP
