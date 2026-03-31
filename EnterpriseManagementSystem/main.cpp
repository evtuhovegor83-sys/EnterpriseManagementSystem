#include <iostream>
#include <sstream>
#include <string>
#include "DatabaseManager.h"
#include "Employee.h"
#include "Part.h"
#include "Order.h"
#include "AuthManager.h"
#include "ReportExporter.h"

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

// ============ ÏÎÑÒÐÀÍÈ×ÍÀß ÍÀÂÈÃÀÖÈß ÄËß ÄÅÒÀËÅÉ ============
void PrintPartsPaginated(DatabaseManager& db) {
    int pageSize = 5;
    int totalCount = Part::GetPartsTotalCount(db);

    if (totalCount == 0) {
        std::cout << "No parts found in database!" << std::endl;
        return;
    }

    int totalPages = (totalCount + pageSize - 1) / pageSize;
    int currentPage = 1;

    do {
        SQLHSTMT hstmt = NULL;
        if (Part::GetPartsPaginated(db, currentPage, pageSize, hstmt)) {
            std::cout << "\n=== Parts List - Page " << currentPage << " of " << totalPages << " (Total: " << totalCount << " parts) ===" << std::endl;
            std::cout << "----------------------------------------------------------------" << std::endl;

            SQLINTEGER id, stock;
            char name[100], number[50];
            double price;

            SQLBindCol(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
            SQLBindCol(hstmt, 2, SQL_C_CHAR, name, sizeof(name), NULL);
            SQLBindCol(hstmt, 3, SQL_C_CHAR, number, sizeof(number), NULL);
            SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &price, 0, NULL);
            SQLBindCol(hstmt, 5, SQL_C_SLONG, &stock, 0, NULL);

            int rowCount = 0;
            while (SQLFetch(hstmt) == SQL_SUCCESS) {
                std::cout << "ID: " << id << " | " << name << " | " << number
                    << " | Price: " << price << " | Stock: " << stock << std::endl;
                rowCount++;
            }

            if (rowCount == 0) {
                std::cout << "No parts found on this page." << std::endl;
            }

            std::cout << "----------------------------------------------------------------" << std::endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }

        std::cout << "\n[N]ext page | [P]revious page | [F]irst page | [L]ast page | [Q]uit: ";
        char choice;
        std::cin >> choice;

        if (choice == 'N' || choice == 'n') {
            if (currentPage < totalPages) {
                currentPage++;
            }
            else {
                std::cout << "You are on the last page!" << std::endl;
            }
        }
        else if (choice == 'P' || choice == 'p') {
            if (currentPage > 1) {
                currentPage--;
            }
            else {
                std::cout << "You are on the first page!" << std::endl;
            }
        }
        else if (choice == 'F' || choice == 'f') {
            currentPage = 1;
            std::cout << "Jumped to first page!" << std::endl;
        }
        else if (choice == 'L' || choice == 'l') {
            currentPage = totalPages;
            std::cout << "Jumped to last page!" << std::endl;
        }
        else if (choice == 'Q' || choice == 'q') {
            break;
        }
    } while (true);
}

void ShowMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  MAIN MENU" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. View all employees" << std::endl;
    std::cout << "2. View all parts" << std::endl;
    std::cout << "3. View orders with details" << std::endl;
    std::cout << "4. View top 5 parts by sales" << std::endl;
    std::cout << "5. Create new order (requires Manager+)" << std::endl;
    std::cout << "6. Add new part (requires Manager+)" << std::endl;
    std::cout << "7. Add new employee (requires Admin only)" << std::endl;
    std::cout << "8. Delete employee (requires Admin only)" << std::endl;
    std::cout << "9. EXPORT: Employees to CSV" << std::endl;
    std::cout << "10. EXPORT: Parts to CSV" << std::endl;
    std::cout << "11. EXPORT: Orders report to CSV" << std::endl;
    std::cout << "12. EXPORT: Top 5 parts to CSV" << std::endl;
    std::cout << "13. EXPORT: Low stock parts to CSV" << std::endl;
    std::cout << "14. View parts with PAGINATION (OFFSET/FETCH)" << std::endl;
    std::cout << "15. Calculate order total (Bonus function #1)" << std::endl;
    std::cout << "16. Get employees by department (Bonus function #2)" << std::endl;
    std::cout << "17. Update part price with auth (Bonus function #3)" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choice: ";
}

int main() {
    setlocale(LC_ALL, "ru");

    std::cout << "========================================" << std::endl;
    std::cout << "  Enterprise Management System v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    DatabaseManager db;
    AuthManager auth;

    if (!db.Connect("DESKTOP-OO16Q6Q\\SQLEXPRESS", "ProductionDB")) {
        std::cout << "ERROR! Failed to connect to database." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "SUCCESS! Connected to database." << std::endl;

    // Àâòîðèçàöèÿ
    std::cout << "\n========== LOGIN ==========" << std::endl;
    std::cout << "Available test users:" << std::endl;
    std::cout << "  - ivanov@company.ru (Administrator)" << std::endl;
    std::cout << "  - petrova@company.ru (Manager)" << std::endl;
    std::cout << "  - kozlovam@company.ru (Warehouse Worker)" << std::endl;
    std::cout << "  - sidorov@company.ru (Accountant)" << std::endl;
    std::cout << "===========================" << std::endl;

    std::string email;
    std::cout << "Enter email: ";
    std::cin >> email;

    if (!auth.Login(db, email)) {
        std::cout << "Login failed. Exiting..." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        std::cin.get();
        return 1;
    }

    int choice;
    do {
        ShowMenu();
        std::cin >> choice;

        switch (choice) {
        case 1:
            PrintAllEmployees(db);
            break;
        case 2:
            PrintAllParts(db);
            break;
        case 3:
            PrintOrdersWithDetails(db);
            break;
        case 4:
            if (auth.CanViewReports()) {
                PrintTop5Parts(db, "2025-03-01", "2025-03-31");
            }
            else {
                std::cout << "Access denied: You don't have permission to view reports!" << std::endl;
            }
            break;
        case 5:
            if (auth.CanEdit()) {
                std::cout << "Creating new order (functionality to be implemented)..." << std::endl;
            }
            else {
                std::cout << "Access denied: Only Manager+ can create orders!" << std::endl;
            }
            break;
        case 6:
            if (auth.CanEdit()) {
                std::cout << "Adding new part (functionality to be implemented)..." << std::endl;
            }
            else {
                std::cout << "Access denied: Only Manager+ can add parts!" << std::endl;
            }
            break;
        case 7:
            if (auth.CanDelete()) {
                std::cout << "Adding new employee (functionality to be implemented)..." << std::endl;
            }
            else {
                std::cout << "Access denied: Only Administrator can add employees!" << std::endl;
            }
            break;
        case 8:
            if (auth.CanDelete()) {
                int empId;
                std::cout << "Enter employee ID to delete: ";
                std::cin >> empId;
                Employee emp;
                if (emp.DeleteWithAuth(db, auth, empId)) {
                    std::cout << "Employee deleted successfully!" << std::endl;
                }
                else {
                    std::cout << "Failed to delete employee!" << std::endl;
                }
            }
            else {
                std::cout << "Access denied: Only Administrator can delete employees!" << std::endl;
            }
            break;
        case 9:
            if (ReportExporter::ExportEmployeesToCSV(db, "Reports/employees.csv")) {
                std::cout << "Employees exported to Reports/employees.csv" << std::endl;
            }
            else {
                std::cout << "Failed to export employees!" << std::endl;
            }
            break;
        case 10:
            if (ReportExporter::ExportPartsToCSV(db, "Reports/parts.csv")) {
                std::cout << "Parts exported to Reports/parts.csv" << std::endl;
            }
            else {
                std::cout << "Failed to export parts!" << std::endl;
            }
            break;
        case 11:
            if (ReportExporter::ExportOrdersReportToCSV(db, "Reports/orders.csv")) {
                std::cout << "Orders exported to Reports/orders.csv" << std::endl;
            }
            else {
                std::cout << "Failed to export orders!" << std::endl;
            }
            break;
        case 12:
            if (auth.CanViewReports()) {
                if (ReportExporter::ExportTop5PartsToCSV(db, "2025-03-01", "2025-03-31", "Reports/top5_parts.csv")) {
                    std::cout << "Top 5 parts exported to Reports/top5_parts.csv" << std::endl;
                }
                else {
                    std::cout << "Failed to export top 5 parts!" << std::endl;
                }
            }
            else {
                std::cout << "Access denied: You don't have permission to export reports!" << std::endl;
            }
            break;
        case 13:
        {
            int threshold;
            std::cout << "Enter stock threshold (e.g., 100): ";
            std::cin >> threshold;
            std::stringstream filename;
            filename << "Reports/low_stock_" << threshold << ".csv";
            if (ReportExporter::ExportLowStockToCSV(db, threshold, filename.str())) {
                std::cout << "Low stock parts exported to " << filename.str() << std::endl;
            }
            else {
                std::cout << "Failed to export low stock parts!" << std::endl;
            }
        }
        break;
        case 14:
            PrintPartsPaginated(db);
            break;
        case 15:
        {
            int orderId;
            std::cout << "Enter order ID: ";
            std::cin >> orderId;
            double total = Order::CalculateOrderTotal(db, orderId);
            std::cout << "Order total: " << total << std::endl;
        }
        break;
        case 16:
        {
            int deptId;
            std::cout << "Enter department ID (1-4): " << std::endl;
            std::cout << "  1 - Assembly shop" << std::endl;
            std::cout << "  2 - Warehouse department" << std::endl;
            std::cout << "  3 - Accounting" << std::endl;
            std::cout << "  4 - Purchasing department" << std::endl;
            std::cout << "Choice: ";
            std::cin >> deptId;

            SQLHSTMT hstmt = NULL;
            if (Employee::GetEmployeesByDepartment(db, deptId, hstmt)) {
                std::cout << "\n=== Employees in Department " << deptId << " ===" << std::endl;
                SQLINTEGER id;
                char name[100], fname[100], email[100];
                double salary;

                SQLBindCol(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
                SQLBindCol(hstmt, 2, SQL_C_CHAR, name, sizeof(name), NULL);
                SQLBindCol(hstmt, 3, SQL_C_CHAR, fname, sizeof(fname), NULL);
                SQLBindCol(hstmt, 4, SQL_C_CHAR, email, sizeof(email), NULL);
                SQLBindCol(hstmt, 5, SQL_C_DOUBLE, &salary, 0, NULL);

                int count = 0;
                while (SQLFetch(hstmt) == SQL_SUCCESS) {
                    std::cout << "ID: " << id << " | " << name << " " << fname
                        << " | " << email << " | Salary: " << salary << std::endl;
                    count++;
                }

                if (count == 0) {
                    std::cout << "No employees found in this department." << std::endl;
                }

                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            }
        }
        break;
        case 17:
        {
            // Ñíà÷àëà ïîêàæåì ñïèñîê äåòàëåé
            SQLHSTMT hstmtParts = NULL;
            if (Part::GetAll(db, hstmtParts)) {
                std::cout << "\n=== Available Parts ===" << std::endl;
                SQLINTEGER pid, pstock;
                char pname[100], pnumber[50];
                double pprice;

                SQLBindCol(hstmtParts, 1, SQL_C_SLONG, &pid, 0, NULL);
                SQLBindCol(hstmtParts, 2, SQL_C_CHAR, pname, sizeof(pname), NULL);
                SQLBindCol(hstmtParts, 3, SQL_C_CHAR, pnumber, sizeof(pnumber), NULL);
                SQLBindCol(hstmtParts, 4, SQL_C_DOUBLE, &pprice, 0, NULL);
                SQLBindCol(hstmtParts, 5, SQL_C_SLONG, &pstock, 0, NULL);

                while (SQLFetch(hstmtParts) == SQL_SUCCESS) {
                    std::cout << "ID: " << pid << " | " << pname << " | Price: " << pprice << std::endl;
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hstmtParts);
            }

            int partId;
            double newPrice;
            std::cout << "\nEnter part ID to update price: ";
            std::cin >> partId;
            std::cout << "Enter new price: ";
            std::cin >> newPrice;

            Part p;
            p.UpdatePriceWithAuth(db, auth, partId, newPrice);
        }
        break;
        case 0:
            std::cout << "Goodbye!" << std::endl;
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
        }
    } while (choice != 0);

    return 0;
}