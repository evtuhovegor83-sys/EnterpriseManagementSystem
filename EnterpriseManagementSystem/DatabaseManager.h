#pragma once
#include <string>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

class DatabaseManager {
private:
    SQLHENV henv;      // Environment handle
    SQLHDBC hdbc;      // Connection handle
    bool connected;

    void PrintODBCError(SQLHANDLE handle, SQLSMALLINT type);

public:
    DatabaseManager();
    ~DatabaseManager();

    bool Connect(const std::wstring& server, const std::wstring& database);
    void Disconnect();
    bool IsConnected() const { return connected; }

    SQLHDBC GetConnection() const { return hdbc; }

    // Выполнение запроса без возврата результата (INSERT, UPDATE, DELETE)
    bool ExecuteNonQuery(const std::wstring& query);

    // Выполнение запроса с возвратом результата (SELECT)
    bool ExecuteQuery(const std::wstring& query, SQLHSTMT& hstmt);
};
