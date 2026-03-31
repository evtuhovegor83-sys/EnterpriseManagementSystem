#include <iostream>
#include "DatabaseManager.h"

int main() {
    setlocale(LC_ALL, "ru");

    std::cout << "========================================" << std::endl;
    std::cout << "  Enterprise Management System v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    DatabaseManager db;

    // Подключение к БД
    if (db.Connect(L"DESKTOP-OO16Q6Q\\SQLEXPRESS", L"ProductionDB")) {
        std::cout << "УСПЕШНО! Подключение к БД установлено." << std::endl;

        // Проверка - выполним простой запрос
        SQLHSTMT hstmt = NULL;
        if (db.ExecuteQuery(L"SELECT COUNT(*) FROM Employees", hstmt)) {
            SQLINTEGER count;
            SQLBindCol(hstmt, 1, SQL_C_SLONG, &count, 0, NULL);
            SQLFetch(hstmt);
            std::wcout << L"Проверка: в таблице Employees найдено " << count << L" записей." << std::endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else {
        std::cout << "ОШИБКА! Не удалось подключиться к БД." << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Нажмите Enter для выхода..." << std::endl;
    std::cin.get();

    return 0;
}