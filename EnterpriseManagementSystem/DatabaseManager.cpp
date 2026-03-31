#include "DatabaseManager.h"
#include <iostream>

void DatabaseManager::PrintODBCError(SQLHANDLE handle, SQLSMALLINT type) {
    SQLWCHAR sqlState[1024];
    SQLWCHAR message[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;

    if (SQLGetDiagRecW(type, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength) == SQL_SUCCESS) {
        std::wcerr << L"ODBC Error: " << message << L" (SQL State: " << sqlState << L")" << std::endl;
    }
}

DatabaseManager::DatabaseManager() : henv(NULL), hdbc(NULL), connected(false) {}

DatabaseManager::~DatabaseManager() {
    Disconnect();
}

bool DatabaseManager::Connect(const std::wstring& server, const std::wstring& database) {
    // 1. Выделяем environment handle
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка выделения environment handle" << std::endl;
        return false;
    }

    // 2. Устанавливаем версию ODBC 3
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка установки версии ODBC" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    // 3. Выделяем connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка выделения connection handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    // 4. Формируем строку подключения
    std::wstring connString = L"DRIVER={ODBC Driver 17 for SQL Server};SERVER=" + server + L";DATABASE=" + database + L";Trusted_Connection=yes;";

    ret = SQLDriverConnectW(hdbc, NULL, (SQLWCHAR*)connString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        connected = true;
        std::wcout << L"Подключено к базе данных: " << database << std::endl;
        return true;
    }
    else {
        std::cerr << "Ошибка подключения к базе данных" << std::endl;
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

bool DatabaseManager::ExecuteNonQuery(const std::wstring& query) {
    if (!connected) return false;

    SQLHSTMT hstmt = NULL;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool DatabaseManager::ExecuteQuery(const std::wstring& query, SQLHSTMT& hstmt) {
    if (!connected) return false;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}