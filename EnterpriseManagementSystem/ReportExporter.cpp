#include "ReportExporter.h"
#include "Order.h"
#include "Part.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

bool ReportExporter::ExportToCSV(SQLHSTMT hstmt, const std::string& filename, const std::string& headers) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл " << filename << std::endl;
        return false;
    }

    // Записываем заголовки
    file << headers << std::endl;

    // Получаем количество столбцов
    SQLSMALLINT numCols = 0;
    SQLNumResultCols(hstmt, &numCols);

    // Для каждого столбца получаем данные
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        std::stringstream line;
        for (SQLSMALLINT i = 1; i <= numCols; i++) {
            char buffer[1024];
            SQLGetData(hstmt, i, SQL_C_CHAR, buffer, sizeof(buffer), NULL);

            if (i > 1) line << ",";

            // Экранируем кавычки и оборачиваем в кавычки если есть запятая или кавычки
            std::string value(buffer);
            if (value.find(',') != std::string::npos || value.find('"') != std::string::npos) {
                size_t pos = 0;
                while ((pos = value.find('"', pos)) != std::string::npos) {
                    value.replace(pos, 1, "\"\"");
                    pos += 2;
                }
                line << "\"" << value << "\"";
            }
            else {
                line << value;
            }
        }
        file << line.str() << std::endl;
    }

    file.close();
    std::cout << "Отчет экспортирован в: " << filename << std::endl;
    return true;
}

bool ReportExporter::ExportEmployeesToCSV(DatabaseManager& db, const std::string& filename) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    std::string query =
        "SELECT EmployeeID, LastName, FirstName, Email, "
        "CONVERT(VARCHAR, HireDate, 23) as HireDate, Salary, "
        "ISNULL(d.DepartmentName, 'No Department') as DepartmentName "
        "FROM Employees e "
        "LEFT JOIN Departments d ON e.DepartmentID = d.DepartmentID";

    if (!db.ExecuteQuery(query, hstmt)) {
        std::cerr << "Ошибка: Не удалось выполнить запрос для экспорта сотрудников" << std::endl;
        return false;
    }

    std::string headers = "EmployeeID,LastName,FirstName,Email,HireDate,Salary,DepartmentName";
    bool result = ExportToCSV(hstmt, filename, headers);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}

bool ReportExporter::ExportPartsToCSV(DatabaseManager& db, const std::string& filename) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    std::string query =
        "SELECT p.PartID, p.PartName, p.PartNumber, p.Price, p.StockQuantity, "
        "ISNULL(w.WarehouseName, 'No Warehouse') as WarehouseName, "
        "ISNULL(s.SupplierName, 'No Supplier') as SupplierName "
        "FROM Parts p "
        "LEFT JOIN Warehouses w ON p.WarehouseID = w.WarehouseID "
        "LEFT JOIN Suppliers s ON p.SupplierID = s.SupplierID";

    if (!db.ExecuteQuery(query, hstmt)) {
        std::cerr << "Ошибка: Не удалось выполнить запрос для экспорта деталей" << std::endl;
        return false;
    }

    std::string headers = "PartID,PartName,PartNumber,Price,StockQuantity,WarehouseName,SupplierName";
    bool result = ExportToCSV(hstmt, filename, headers);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}

bool ReportExporter::ExportOrdersReportToCSV(DatabaseManager& db, const std::string& filename) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    if (!Order::GetAllOrdersWithDetails(db, hstmt)) {
        std::cerr << "Ошибка: Не удалось выполнить запрос для экспорта заказов" << std::endl;
        return false;
    }

    std::string headers = "OrderID,OrderDate,Status,EmployeeName,TotalItems,TotalAmount";
    bool result = ExportToCSV(hstmt, filename, headers);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}

bool ReportExporter::ExportTop5PartsToCSV(DatabaseManager& db, const std::string& startDate, const std::string& endDate, const std::string& filename) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    if (!Order::GetTop5PartsBySales(db, startDate, endDate, hstmt)) {
        std::cerr << "Ошибка: Не удалось выполнить запрос для экспорта топ-5 деталей" << std::endl;
        return false;
    }

    std::stringstream headers;
    headers << "PartID,PartName,PartNumber,TotalSold,TotalRevenue,Period: " << startDate << " - " << endDate;
    bool result = ExportToCSV(hstmt, filename, headers.str());

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}

bool ReportExporter::ExportLowStockToCSV(DatabaseManager& db, int threshold, const std::string& filename) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    if (!Part::GetLowStock(db, threshold, hstmt)) {
        std::cerr << "Ошибка: Не удалось выполнить запрос для экспорта деталей с низким остатком" << std::endl;
        return false;
    }

    std::stringstream headers;
    headers << "PartID,PartName,PartNumber,Price,StockQuantity (Порог: " << threshold << ")";
    bool result = ExportToCSV(hstmt, filename, headers.str());

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return result;
}