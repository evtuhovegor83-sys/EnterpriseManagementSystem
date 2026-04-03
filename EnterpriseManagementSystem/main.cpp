#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <windows.h>
#include <ctime>
#include "DatabaseManager.h"
#include "Employee.h"
#include "Part.h"
#include "Order.h"
#include "AuthManager.h"
#include "ReportExporter.h"

// ============ НАСТРОЙКА КОНСОЛИ ДЛЯ РУССКОГО ЯЗЫКА ============
void SetupRussianConsole() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
}

// ============ ФУНКЦИИ ДЛЯ БЕЗОПАСНОГО ВВОДА (БЕСКОНЕЧНЫЕ ПОПЫТКИ) ============

int SafeInputInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(10000, '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Ошибка! Введите целое число." << std::endl;
    }
}

double SafeInputDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(10000, '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Ошибка! Введите число (например, 123.45)." << std::endl;
    }
}

std::string SafeInputString(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}

// ============ ФУНКЦИИ ВЫВОДА ============

void PrintAllEmployees(DatabaseManager& db) {
    SQLHSTMT hstmt = NULL;
    if (Employee::GetAll(db, hstmt)) {
        std::cout << "\n=== Список сотрудников ===" << std::endl;

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
                << " | " << email << " | Зарплата: " << salary << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

void PrintAllParts(DatabaseManager& db) {
    SQLHSTMT hstmt = NULL;
    if (Part::GetAll(db, hstmt)) {
        std::cout << "\n=== Список деталей ===" << std::endl;

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
                << " | Цена: " << price << " | Остаток: " << stock << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

void PrintOrdersWithDetails(DatabaseManager& db) {
    SQLHSTMT hstmt = NULL;
    if (Order::GetAllOrdersWithDetails(db, hstmt)) {
        std::cout << "\n=== Заказы с деталями (JOIN запрос) ===" << std::endl;

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
            std::cout << "ID заказа: " << orderID << " | Дата: " << orderDate
                << " | Статус: " << status << " | Сотрудник: " << employeeName
                << " | Кол-во позиций: " << totalItems << " | Сумма: " << totalAmount << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

void PrintTop5Parts(DatabaseManager& db, const std::string& startDate, const std::string& endDate) {
    SQLHSTMT hstmt = NULL;
    if (Order::GetTop5PartsBySales(db, startDate, endDate, hstmt)) {
        std::cout << "\n=== Топ-5 деталей по продажам (" << startDate << " - " << endDate << ") ===" << std::endl;

        SQLINTEGER partID, totalSold;
        char partName[100], partNumber[50];
        double totalRevenue;

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &partID, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, partName, sizeof(partName), NULL);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, partNumber, sizeof(partNumber), NULL);
        SQLBindCol(hstmt, 4, SQL_C_SLONG, &totalSold, 0, NULL);
        SQLBindCol(hstmt, 5, SQL_C_DOUBLE, &totalRevenue, 0, NULL);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "ID: " << partID << " | " << partName << " | Продано: " << totalSold
                << " | Выручка: " << totalRevenue << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
}

// ============ ПОСТРАНИЧНАЯ НАВИГАЦИЯ ДЛЯ ДЕТАЛЕЙ ============
void PrintPartsPaginated(DatabaseManager& db) {
    int pageSize = 5;
    int totalCount = Part::GetPartsTotalCount(db);

    if (totalCount == 0) {
        std::cout << "Детали не найдены в базе данных!" << std::endl;
        return;
    }

    int totalPages = (totalCount + pageSize - 1) / pageSize;
    int currentPage = 1;

    do {
        SQLHSTMT hstmt = NULL;
        if (Part::GetPartsPaginated(db, currentPage, pageSize, hstmt)) {
            std::cout << "\n=== Список деталей - Страница " << currentPage << " из " << totalPages << " (Всего: " << totalCount << " деталей) ===" << std::endl;
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
                    << " | Цена: " << price << " | Остаток: " << stock << std::endl;
                rowCount++;
            }

            if (rowCount == 0) {
                std::cout << "На этой странице нет деталей." << std::endl;
            }

            std::cout << "----------------------------------------------------------------" << std::endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }

        std::cout << "\n[С]ледующая страница | [П]редыдущая страница | [Н]ачало | [К]онец | [В]ыход: ";
        char choice;
        std::cin >> choice;
        choice = static_cast<char>(toupper(choice));

        if (choice == 'С') {
            if (currentPage < totalPages) {
                currentPage++;
            }
            else {
                std::cout << "Вы на последней странице!" << std::endl;
            }
        }
        else if (choice == 'П') {
            if (currentPage > 1) {
                currentPage--;
            }
            else {
                std::cout << "Вы на первой странице!" << std::endl;
            }
        }
        else if (choice == 'Н') {
            currentPage = 1;
            std::cout << "Переход на первую страницу!" << std::endl;
        }
        else if (choice == 'К') {
            currentPage = totalPages;
            std::cout << "Переход на последнюю страницу!" << std::endl;
        }
        else if (choice == 'В') {
            break;
        }
    } while (true);
}

void ShowMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  ГЛАВНОЕ МЕНЮ" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Просмотр всех сотрудников" << std::endl;
    std::cout << "2. Просмотр всех деталей" << std::endl;
    std::cout << "3. Просмотр заказов с деталями" << std::endl;
    std::cout << "4. Просмотр топ-5 деталей по продажам" << std::endl;
    std::cout << "5. Создать новый заказ (требуется Менеджер+)" << std::endl;
    std::cout << "6. Добавить новую деталь (требуется Менеджер+)" << std::endl;
    std::cout << "7. Добавить нового сотрудника (только Админ)" << std::endl;
    std::cout << "8. Удалить сотрудника (только Админ)" << std::endl;
    std::cout << "9. ЭКСПОРТ: Сотрудники в CSV" << std::endl;
    std::cout << "10. ЭКСПОРТ: Детали в CSV" << std::endl;
    std::cout << "11. ЭКСПОРТ: Отчет по заказам в CSV" << std::endl;
    std::cout << "12. ЭКСПОРТ: Топ-5 деталей в CSV" << std::endl;
    std::cout << "13. ЭКСПОРТ: Детали с низким остатком в CSV" << std::endl;
    std::cout << "14. Просмотр деталей с постраничной навигацией (OFFSET/FETCH)" << std::endl;
    std::cout << "15. Рассчитать общую стоимость заказа (Бонус #1)" << std::endl;
    std::cout << "16. Получить сотрудников по отделу (Бонус #2)" << std::endl;
    std::cout << "17. Обновить цену детали (требуется Менеджер+) (Бонус #3)" << std::endl;
    std::cout << "18. Статистика по складам (Бонус #4)" << std::endl;
    std::cout << "19. Отменить заказ с возвратом товаров (Бонус #5)" << std::endl;
    std::cout << "0. Выход" << std::endl;
}

// ============ ФУНКЦИЯ АВТОРИЗАЦИИ (БЕСКОНЕЧНЫЕ ПОПЫТКИ) ============
bool LoginWithRetry(DatabaseManager& db, AuthManager& auth) {
    std::cout << "\n========== ВХОД В СИСТЕМУ ==========" << std::endl;
    std::cout << "Доступные тестовые пользователи:" << std::endl;
    std::cout << "  - ivanov@company.ru (Администратор)" << std::endl;
    std::cout << "  - petrova@company.ru (Менеджер)" << std::endl;
    std::cout << "  - kozlovam@company.ru (Складской работник)" << std::endl;
    std::cout << "  - sidorov@company.ru (Бухгалтер)" << std::endl;
    std::cout << "===================================" << std::endl;

    while (true) {
        std::cout << "\nВведите email: ";
        std::string email;
        std::cin >> email;
        std::cin.ignore(10000, '\n');

        if (auth.Login(db, email)) {
            return true;
        }

        std::cout << "Ошибка: Пользователь с email '" << email << "' не найден." << std::endl;
        std::cout << "Попробуйте еще раз." << std::endl;
    }
}

int main() {
    SetupRussianConsole();
    setlocale(LC_ALL, "ru");

    std::cout << "========================================" << std::endl;
    std::cout << "  Enterprise Management System v1.0" << std::endl;
    std::cout << "  Система управления предприятием" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    DatabaseManager db;
    AuthManager auth;

    if (!db.Connect("DESKTOP-OO16Q6Q\\SQLEXPRESS", "ProductionDB")) {
        std::cout << "ОШИБКА! Не удалось подключиться к базе данных." << std::endl;
        std::cout << "Нажмите Enter для выхода..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "УСПЕШНО! Подключение к базе данных установлено." << std::endl;

    if (!LoginWithRetry(db, auth)) {
        return 1;
    }

    int choice;
    do {
        ShowMenu();
        choice = SafeInputInt("Выберите пункт меню: ");

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
                std::cout << "Доступ запрещен: у вас нет прав для просмотра отчетов!" << std::endl;
            }
            break;
        case 5:
            if (auth.CanEdit()) {
                Order::CreateNewOrder(db, auth);
            }
            else {
                std::cout << "Доступ запрещен: только Менеджер+ может создавать заказы!" << std::endl;
            }
            break;
        case 6:
            if (auth.CanEdit()) {
                Part::AddNewPart(db, auth);
            }
            else {
                std::cout << "Доступ запрещен: только Менеджер+ может добавлять детали!" << std::endl;
            }
            break;
        case 7:
            if (auth.CanDelete()) {
                Employee::AddNewEmployee(db, auth);
            }
            else {
                std::cout << "Доступ запрещен: только Администратор может добавлять сотрудников!" << std::endl;
            }
            break;
        case 8:
            if (auth.CanDelete()) {
                int empId = SafeInputInt("Введите ID сотрудника для удаления: ");
                Employee emp;
                if (emp.DeleteWithAuth(db, auth, empId)) {
                    std::cout << "Сотрудник успешно удален!" << std::endl;
                }
                else {
                    std::cout << "Не удалось удалить сотрудника!" << std::endl;
                }
            }
            else {
                std::cout << "Доступ запрещен: только Администратор может удалять сотрудников!" << std::endl;
            }
            break;
        case 9:
            if (ReportExporter::ExportEmployeesToCSV(db, "Reports/employees.csv")) {
                std::cout << "Сотрудники экспортированы в Reports/employees.csv" << std::endl;
            }
            else {
                std::cout << "Не удалось экспортировать сотрудников!" << std::endl;
            }
            break;
        case 10:
            if (ReportExporter::ExportPartsToCSV(db, "Reports/parts.csv")) {
                std::cout << "Детали экспортированы в Reports/parts.csv" << std::endl;
            }
            else {
                std::cout << "Не удалось экспортировать детали!" << std::endl;
            }
            break;
        case 11:
            if (ReportExporter::ExportOrdersReportToCSV(db, "Reports/orders.csv")) {
                std::cout << "Заказы экспортированы в Reports/orders.csv" << std::endl;
            }
            else {
                std::cout << "Не удалось экспортировать заказы!" << std::endl;
            }
            break;
        case 12:
            if (auth.CanViewReports()) {
                if (ReportExporter::ExportTop5PartsToCSV(db, "2025-03-01", "2025-03-31", "Reports/top5_parts.csv")) {
                    std::cout << "Топ-5 деталей экспортированы в Reports/top5_parts.csv" << std::endl;
                }
                else {
                    std::cout << "Не удалось экспортировать топ-5 деталей!" << std::endl;
                }
            }
            else {
                std::cout << "Доступ запрещен: у вас нет прав для экспорта отчетов!" << std::endl;
            }
            break;
        case 13:
        {
            int threshold = SafeInputInt("Введите порог остатка (например, 100): ");
            std::stringstream filename;
            filename << "Reports/low_stock_" << threshold << ".csv";
            if (ReportExporter::ExportLowStockToCSV(db, threshold, filename.str())) {
                std::cout << "Детали с низким остатком экспортированы в " << filename.str() << std::endl;
            }
            else {
                std::cout << "Не удалось экспортировать детали с низким остатком!" << std::endl;
            }
        }
        break;
        case 14:
            PrintPartsPaginated(db);
            break;
        case 15:
        {
            int orderId = SafeInputInt("Введите ID заказа: ");
            double total = Order::CalculateOrderTotal(db, orderId);
            std::cout << "Общая стоимость заказа: " << total << std::endl;
        }
        break;
        case 16:
        {
            std::cout << "\nВарианты отделов:" << std::endl;
            std::cout << "  1 - Сборочный цех" << std::endl;
            std::cout << "  2 - Складской отдел" << std::endl;
            std::cout << "  3 - Бухгалтерия" << std::endl;
            std::cout << "  4 - Отдел закупок" << std::endl;
            int deptId = SafeInputInt("Введите ID отдела (1-4): ");

            if (deptId < 1 || deptId > 4) {
                std::cout << "Неверный ID отдела! Введите число от 1 до 4." << std::endl;
                break;
            }

            SQLHSTMT hstmt = NULL;
            if (Employee::GetEmployeesByDepartment(db, deptId, hstmt)) {
                std::cout << "\n=== Сотрудники отдела " << deptId << " ===" << std::endl;
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
                        << " | " << email << " | Зарплата: " << salary << std::endl;
                    count++;
                }

                if (count == 0) {
                    std::cout << "В этом отделе нет сотрудников." << std::endl;
                }

                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            }
        }
        break;
        case 17:
        {
            SQLHSTMT hstmtParts = NULL;
            if (Part::GetAll(db, hstmtParts)) {
                std::cout << "\n=== Доступные детали ===" << std::endl;
                SQLINTEGER pid, pstock;
                char pname[100], pnumber[50];
                double pprice;

                SQLBindCol(hstmtParts, 1, SQL_C_SLONG, &pid, 0, NULL);
                SQLBindCol(hstmtParts, 2, SQL_C_CHAR, pname, sizeof(pname), NULL);
                SQLBindCol(hstmtParts, 3, SQL_C_CHAR, pnumber, sizeof(pnumber), NULL);
                SQLBindCol(hstmtParts, 4, SQL_C_DOUBLE, &pprice, 0, NULL);
                SQLBindCol(hstmtParts, 5, SQL_C_SLONG, &pstock, 0, NULL);

                while (SQLFetch(hstmtParts) == SQL_SUCCESS) {
                    std::cout << "ID: " << pid << " | " << pname << " | Цена: " << pprice << std::endl;
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hstmtParts);
            }

            int partId = SafeInputInt("\nВведите ID детали для обновления цены: ");
            double newPrice = SafeInputDouble("Введите новую цену: ");

            // Вызываем статический метод без создания объекта
            Part::UpdatePriceWithAuth(db, auth, partId, newPrice);
        }
        break;
        case 18:
            Part::GetWarehouseStatistics(db);
            break;
        case 19:
        {
            SQLHSTMT hstmtOrders = NULL;
            std::string activeQuery = "SELECT OrderID, OrderDate, Status FROM Orders WHERE Status != 'Cancelled' AND Status != 'Выполнен' AND Status != 'Completed'";
            if (db.ExecuteQuery(activeQuery, hstmtOrders)) {
                std::cout << "\n=== Активные заказы ===" << std::endl;
                SQLINTEGER oid;
                char odate[20], ostatus[20];
                SQLBindCol(hstmtOrders, 1, SQL_C_SLONG, &oid, 0, NULL);
                SQLBindCol(hstmtOrders, 2, SQL_C_CHAR, odate, sizeof(odate), NULL);
                SQLBindCol(hstmtOrders, 3, SQL_C_CHAR, ostatus, sizeof(ostatus), NULL);

                int count = 0;
                while (SQLFetch(hstmtOrders) == SQL_SUCCESS) {
                    std::cout << "ID: " << oid << " | Дата: " << odate << " | Статус: " << ostatus << std::endl;
                    count++;
                }

                if (count == 0) {
                    std::cout << "Активных заказов не найдено." << std::endl;
                }
                SQLFreeHandle(SQL_HANDLE_STMT, hstmtOrders);
            }

            int orderId = SafeInputInt("\nВведите ID заказа для отмены: ");
            Order o;
            o.CancelOrderWithRestore(db, orderId);
        }
        break;
        case 0:
            std::cout << "До свидания!" << std::endl;
            break;
        default:
            std::cout << "Неверный выбор! Пожалуйста, введите число от 0 до 19." << std::endl;
        }
    } while (choice != 0);

    return 0;
}