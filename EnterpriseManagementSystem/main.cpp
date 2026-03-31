#include <iostream>
#include "DatabaseManager.h"
#include "Employee.h"
#include "Part.h"
#include "Order.h"

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

void PrintAllParts(DatabaseManager& db) {
    SQLHSTMT hstmt = NULL;
    if (Part::GetAll(db, hstmt)) {
        std::cout << "\n=== Parts List ===" << std::endl;

        SQLINTEGER id, stock;
        char name[100], number[50];
        double price;

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, name, sizeof(name), NULL);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, number, sizeof(number), NULL);
        SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &price, 0, NULL);
        SQLBindCol(hstmt, 5, SQL_C_SLONG, &stock, 0, NULL);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "ID: " << id << " | " << name << " | " << number
                << " | Price: " << price << " | Stock: " << stock << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

void PrintOrdersWithDetails(DatabaseManager& db) {
    SQLHSTMT hstmt = NULL;
    if (Order::GetAllOrdersWithDetails(db, hstmt)) {
        std::cout << "\n=== Orders with Details (JOIN query) ===" << std::endl;

        SQLINTEGER orderID, totalItems;
        char orderDate[20], status[20], employeeName[100];
        double totalAmount;

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &orderID, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, orderDate, sizeof(orderDate), NULL);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, status, sizeof(status), NULL);
        SQLBindCol(hstmt, 4, SQL_C_CHAR, employeeName, sizeof(employeeName), NULL);
        SQLBindCol(hstmt, 5, SQL_C_SLONG, &totalItems, 0, NULL);
        SQLBindCol(hstmt, 6, SQL_C_DOUBLE, &totalAmount, 0, NULL);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "OrderID: " << orderID << " | Date: " << orderDate
                << " | Status: " << status << " | Employee: " << employeeName
                << " | Items: " << totalItems << " | Amount: " << totalAmount << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

void PrintTop5Parts(DatabaseManager& db, const std::string& startDate, const std::string& endDate) {
    SQLHSTMT hstmt = NULL;
    if (Order::GetTop5PartsBySales(db, startDate, endDate, hstmt)) {
        std::cout << "\n=== Top 5 Parts by Sales (" << startDate << " - " << endDate << ") ===" << std::endl;

        SQLINTEGER partID, totalSold;
        char partName[100], partNumber[50];
        double totalRevenue;

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &partID, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, partName, sizeof(partName), NULL);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, partNumber, sizeof(partNumber), NULL);
        SQLBindCol(hstmt, 4, SQL_C_SLONG, &totalSold, 0, NULL);
        SQLBindCol(hstmt, 5, SQL_C_DOUBLE, &totalRevenue, 0, NULL);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "ID: " << partID << " | " << partName << " | Sold: " << totalSold
                << " | Revenue: " << totalRevenue << std::endl;
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

        // ============ DEMO 1: Employee CRUD ============
        std::cout << "\n========== DEMO 1: Employee CRUD ==========" << std::endl;

        Employee newEmp;
        newEmp.SetLastName("Smirnov");
        newEmp.SetFirstName("Andrey");
        newEmp.SetMiddleName("Nikolaevich");
        newEmp.SetEmail("andrey.smirnov@company.ru");
        newEmp.SetSalary(75000);
        newEmp.SetDepartmentID(1);

        if (newEmp.Create(db)) {
            std::cout << "[OK] New employee added: Smirnov Andrey" << std::endl;
        }

        PrintAllEmployees(db);

        // ============ DEMO 2: Part CRUD ============
        std::cout << "\n========== DEMO 2: Part CRUD ==========" << std::endl;

        Part newPart;
        newPart.SetPartName("Bolt M8");
        newPart.SetPartNumber("BLT-008");
        newPart.SetPrice(25.50);
        newPart.SetStockQuantity(500);
        newPart.SetWarehouseID(1);
        newPart.SetSupplierID(1);

        if (newPart.Create(db)) {
            std::cout << "[OK] New part added: Bolt M8" << std::endl;
        }

        PrintAllParts(db);

        // ============ DEMO 3: Complex JOIN query ============
        std::cout << "\n========== DEMO 3: Complex JOIN (Parts with Warehouse and Supplier) ==========" << std::endl;

        SQLHSTMT hstmt = NULL;
        if (Part::GetPartsWithDetails(db, hstmt)) {
            SQLINTEGER id, stock;
            char name[100], number[50], warehouseName[100], warehouseLoc[100], supplierName[100], phone[20], email[100];
            double price;

            SQLBindCol(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
            SQLBindCol(hstmt, 2, SQL_C_CHAR, name, sizeof(name), NULL);
            SQLBindCol(hstmt, 3, SQL_C_CHAR, number, sizeof(number), NULL);
            SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &price, 0, NULL);
            SQLBindCol(hstmt, 5, SQL_C_SLONG, &stock, 0, NULL);
            SQLBindCol(hstmt, 6, SQL_C_CHAR, warehouseName, sizeof(warehouseName), NULL);
            SQLBindCol(hstmt, 7, SQL_C_CHAR, warehouseLoc, sizeof(warehouseLoc), NULL);
            SQLBindCol(hstmt, 8, SQL_C_CHAR, supplierName, sizeof(supplierName), NULL);
            SQLBindCol(hstmt, 9, SQL_C_CHAR, phone, sizeof(phone), NULL);
            SQLBindCol(hstmt, 10, SQL_C_CHAR, email, sizeof(email), NULL);

            while (SQLFetch(hstmt) == SQL_SUCCESS) {
                std::cout << "Part: " << name << " | Stock: " << stock
                    << " | Warehouse: " << warehouseName
                    << " | Supplier: " << supplierName << std::endl;
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }

        // ============ DEMO 4: Order with stored procedure ============
        std::cout << "\n========== DEMO 4: Create Order with Stored Procedure (sp_CreateOrder) ==========" << std::endl;

        Order newOrder;
        std::vector<OrderPartItem> items;

        // Заказываем 50 винтов (PartID=1) и 30 гаек (PartID=2)
        OrderPartItem item1;
        item1.partID = 1;
        item1.quantity = 50;
        items.push_back(item1);

        OrderPartItem item2;
        item2.partID = 2;
        item2.quantity = 30;
        items.push_back(item2);

        if (newOrder.CreateWithProcedure(db, 2, "2025-03-25", items)) {
            std::cout << "[OK] Order created successfully!" << std::endl;
        }
        else {
            std::cout << "[FAIL] Failed to create order" << std::endl;
        }

        // ============ DEMO 5: Orders with JOIN ============
        PrintOrdersWithDetails(db);

        // ============ DEMO 6: Top 5 Parts by Sales (Window Function) ============
        PrintTop5Parts(db, "2025-03-01", "2025-03-31");

    }
    else {
        std::cout << "ERROR! Failed to connect to database." << std::endl;
    }

    std::cout << std::endl << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}