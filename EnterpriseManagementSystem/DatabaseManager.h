#pragma once
#include <string>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

class DatabaseManager {
private:
    SQLHENV henv;
    SQLHDBC hdbc;
    bool connected;

    void PrintODBCError(SQLHANDLE handle, SQLSMALLINT type);

public:
    DatabaseManager();
    ~DatabaseManager();

    bool Connect(const std::string& server, const std::string& database);
    void Disconnect();
    bool IsConnected() const { return connected; }

    SQLHDBC GetConnection() const { return hdbc; }

    bool ExecuteNonQuery(const std::string& query);
    bool ExecuteQuery(const std::string& query, SQLHSTMT& hstmt);
};