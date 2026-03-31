#include <iostream>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

// Функция для вывода ошибок ODBC
void PrintODBCError(SQLHANDLE handle, SQLSMALLINT type) {
    SQLWCHAR sqlState[1024];
    SQLWCHAR message[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;

    if (SQLGetDiagRecW(type, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength) == SQL_SUCCESS) {
        std::wcout << L"ODBC Error: " << message << L" (SQL State: " << sqlState << L")" << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "ru");

    SQLHENV henv = NULL;   // Environment handle
    SQLHDBC hdbc = NULL;   // Connection handle
    SQLHSTMT hstmt = NULL; // Statement handle
    SQLRETURN ret;

    std::cout << "========================================" << std::endl;
    std::cout << "  Enterprise Management System v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // 1. Выделяем environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Ошибка выделения environment handle" << std::endl;
        return 1;
    }

    // 2. Устанавливаем версию ODBC 3
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Ошибка установки версии ODBC" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // 3. Выделяем connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Ошибка выделения connection handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // 4. Подключаемся к SQL Server
    std::wstring connString = L"DRIVER={ODBC Driver 17 for SQL Server};SERVER=DESKTOP-OO16Q6Q\\SQLEXPRESS;DATABASE=ProductionDB;Trusted_Connection=yes;";

    std::wcout << L"Подключение к базе данных ProductionDB..." << std::endl;

    ret = SQLDriverConnectW(hdbc, NULL, (SQLWCHAR*)connString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        std::cout << "УСПЕШНО! Подключение к БД установлено." << std::endl;
        std::cout << std::endl;

        // 5. Проверка - выполним простой запрос
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            SQLWCHAR query[] = L"SELECT COUNT(*) FROM Employees";
            ret = SQLExecDirectW(hstmt, query, SQL_NTS);

            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                SQLINTEGER count;
                SQLBindCol(hstmt, 1, SQL_C_SLONG, &count, 0, NULL);
                SQLFetch(hstmt);
                std::wcout << L"Проверка: в таблице Employees найдено " << count << L" записей." << std::endl;
            }

            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else {
        std::cout << "ОШИБКА! Не удалось подключиться к БД." << std::endl;
        PrintODBCError(hdbc, SQL_HANDLE_DBC);
    }

    // 6. Закрываем соединение и освобождаем ресурсы
    if (hdbc) {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    if (henv) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }

    std::cout << std::endl;
    std::cout << "Нажмите Enter для выхода..." << std::endl;
    std::cin.get();

    return 0;
}