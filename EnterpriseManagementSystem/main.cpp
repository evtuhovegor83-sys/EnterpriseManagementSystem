#include <iostream>
#include "DatabaseManager.h"
#include "Employee.h"

void PrintAllEmployees(DatabaseManager& db) {
    SQLHSTMT hstmt = NULL;
    if (Employee::GetAll(db, hstmt)) {
        std::cout << "\n=== Employee List ===" << std::endl;

        SQLINTEGER id;
        char lastName[100], firstName[100], email[100];
        double salary;

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, lastName, sizeof(lastName), NULL);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, firstName, sizeof(firstName), NULL);
        SQLBindCol(hstmt, 4, SQL_C_CHAR, email, sizeof(email), NULL);
        SQLBindCol(hstmt, 5, SQL_C_DOUBLE, &salary, 0, NULL);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "ID: " << id << " | " << lastName << " " << firstName
                << " | " << email << " | Salary: " << salary << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

int main() {
    setlocale(LC_ALL, "ru");

    std::cout << "========================================" << std::endl;
    std::cout << "  Enterprise Management System v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    DatabaseManager db;

    if (db.Connect("DESKTOP-OO16Q6Q\\SQLEXPRESS", "ProductionDB")) {
        std::cout << "SUCCESS! Connected to database." << std::endl;

        Employee newEmp;
        newEmp.SetLastName("Testov");
        newEmp.SetFirstName("Petr");
        newEmp.SetMiddleName("Ivanovich");
        newEmp.SetEmail("test@company.ru");
        newEmp.SetSalary(60000);
        newEmp.SetDepartmentID(2);

        if (newEmp.Create(db)) {
            std::cout << "\n[OK] New employee added!" << std::endl;
        }
        else {
            std::cout << "\n[FAIL] Error adding employee" << std::endl;
        }

        PrintAllEmployees(db);

        SQLHSTMT hstmt = NULL;
        std::cout << "\n=== Search (salary 70000-100000) ===" << std::endl;
        if (Employee::Search(db, "", 70000, 100000, hstmt)) {
            SQLINTEGER id;
            char lastName[100], firstName[100], email[100], deptName[100];
            double salary;

            SQLBindCol(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
            SQLBindCol(hstmt, 2, SQL_C_CHAR, lastName, sizeof(lastName), NULL);
            SQLBindCol(hstmt, 3, SQL_C_CHAR, firstName, sizeof(firstName), NULL);
            SQLBindCol(hstmt, 4, SQL_C_CHAR, email, sizeof(email), NULL);
            SQLBindCol(hstmt, 5, SQL_C_DOUBLE, &salary, 0, NULL);
            SQLBindCol(hstmt, 6, SQL_C_CHAR, deptName, sizeof(deptName), NULL);

            while (SQLFetch(hstmt) == SQL_SUCCESS) {
                std::cout << "ID: " << id << " | " << lastName << " " << firstName
                    << " | Salary: " << salary << " | Dept: " << deptName << std::endl;
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }

    }
    else {
        std::cout << "ERROR! Failed to connect to database." << std::endl;
    }

    std::cout << std::endl << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}