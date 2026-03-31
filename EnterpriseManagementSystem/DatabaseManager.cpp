#include "DatabaseManager.h"
#include <iostream>
#include <string>

void DatabaseManager::PrintODBCError(SQLHANDLE handle, SQLSMALLINT type) {
    SQLWCHAR sqlState[1024];
    SQLWCHAR message[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;

    if (SQLGetDiagRecW(type, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength) == SQL_SUCCESS) {
        std::wcerr << L"ODBC Error: " << message << std::endl;
    }
}

DatabaseManager::DatabaseManager() : henv(NULL), hdbc(NULL), connected(false) {}

DatabaseManager::~DatabaseManager() {
    Disconnect();
}

bool DatabaseManager::Connect(const std::string& server, const std::string& database) {
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment" << std::endl;
        return false;
    }

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error setting ODBC version" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating connection" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    std::string connString = "DRIVER={ODBC Driver 17 for SQL Server};SERVER=" + server + ";DATABASE=" + database + ";Trusted_Connection=yes;";

    std::wstring wConnString(connString.begin(), connString.end());

    ret = SQLDriverConnectW(hdbc, NULL, (SQLWCHAR*)wConnString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        connected = true;
        std::cout << "Connected to database: " << database << std::endl;
        return true;
    }
    else {
        std::cerr << "Connection failed" << std::endl;
        PrintODBCError(hdbc, SQL_HANDLE_DBC);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }
}

void DatabaseManager::Disconnect() {
    if (hdbc) {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        hdbc = NULL;
    }
    if (henv) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        henv = NULL;
    }
    connected = false;
}

bool DatabaseManager::ExecuteNonQuery(const std::string& query) {
    if (!connected) return false;

    SQLHSTMT hstmt = NULL;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    std::wstring wQuery(query.begin(), query.end());
    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool DatabaseManager::ExecuteQuery(const std::string& query, SQLHSTMT& hstmt) {
    if (!connected) return false;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    std::wstring wQuery(query.begin(), query.end());
    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}