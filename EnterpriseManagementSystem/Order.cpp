#include "Order.h"
#include "Part.h"
#include "AuthManager.h"
#include <iostream>
#include <sstream>
#include <ctime>

// Прототип функции для безопасного ввода (объявлена в main.cpp)
int SafeInputInt(const std::string& prompt);

Order::Order() : orderID(-1), employeeID(-1), status("") {}

Order::Order(int employeeID, const std::string& orderDate)
    : orderID(-1), employeeID(employeeID), orderDate(orderDate), status("New") {
}

bool Order::CreateWithProcedure(DatabaseManager& db, int employeeID, const std::string& orderDate, const std::vector<OrderPartItem>& items) {
    if (!db.IsConnected()) return false;

    // Ограничиваем до 3 деталей (как в нашей хранимой процедуре)
    if (items.empty() || items.size() > 3) {
        std::cerr << "Ошибка: Заказ должен содержать от 1 до 3 деталей" << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << "{CALL sp_CreateOrder(";
    ss << employeeID << ", ";
    ss << "'" << orderDate << "', ";

    // Передаем параметры для до 3 деталей
    for (size_t i = 0; i < 3; i++) {
        if (i < items.size()) {
            ss << items[i].partID << ", " << items[i].quantity;
        }
        else {
            ss << "NULL, NULL";
        }

        if (i < 2) {
            ss << ", ";
        }
    }
    ss << ")}";

    std::string query = ss.str();
    std::cout << "Выполнение: " << query << std::endl;

    SQLHSTMT hstmt = NULL;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, db.GetConnection(), &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    // Используем ANSI версию вместо Wide
    ret = SQLExecDirectA(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        // Получаем результат
        SQLINTEGER resultOrderID;
        char message[256];

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &resultOrderID, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, message, sizeof(message), NULL);

        if (SQLFetch(hstmt) == SQL_SUCCESS) {
            orderID = resultOrderID;
            std::cout << "Результат: " << message << " (ID заказа=" << orderID << ")" << std::endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return true;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return false;
}

bool Order::GetAllOrdersWithDetails(DatabaseManager& db, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    // Сложный JOIN: Orders + Employees + OrderParts + Parts (более 3 таблиц)
    std::string query =
        "SELECT o.OrderID, o.OrderDate, o.Status, "
        "e.LastName + ' ' + e.FirstName AS EmployeeName, "
        "COUNT(op.PartID) AS TotalItems, "
        "SUM(op.Quantity * p.Price) AS TotalAmount "
        "FROM Orders o "
        "INNER JOIN Employees e ON o.EmployeeID = e.EmployeeID "
        "INNER JOIN OrderParts op ON o.OrderID = op.OrderID "
        "INNER JOIN Parts p ON op.PartID = p.PartID "
        "GROUP BY o.OrderID, o.OrderDate, o.Status, e.LastName, e.FirstName "
        "ORDER BY o.OrderDate DESC";

    return db.ExecuteQuery(query, hstmt);
}

bool Order::GetTop5PartsBySales(DatabaseManager& db, const std::string& startDate, const std::string& endDate, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "{CALL sp_Top5PartsBySales('" << startDate << "', '" << endDate << "')}";

    std::string query = ss.str();

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, db.GetConnection(), &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    // Используем ANSI версию вместо Wide
    ret = SQLExecDirectA(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool Order::GetOrdersByEmployee(DatabaseManager& db, int employeeID, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "SELECT OrderID, OrderDate, Status FROM Orders WHERE EmployeeID = " << employeeID << " ORDER BY OrderDate DESC";

    return db.ExecuteQuery(ss.str(), hstmt);
}

bool Order::UpdateStatus(DatabaseManager& db, int orderID, const std::string& newStatus) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "UPDATE Orders SET Status = '" << newStatus << "' WHERE OrderID = " << orderID;

    return db.ExecuteNonQuery(ss.str());
}

// ============ ДОПОЛНИТЕЛЬНАЯ ФУНКЦИЯ 1: Расчет общей стоимости заказа ============

double Order::CalculateOrderTotal(DatabaseManager& db, int orderID) {
    if (!db.IsConnected()) return 0.0;

    SQLHSTMT hstmt = NULL;
    std::stringstream ss;
    ss << "SELECT SUM(op.Quantity * p.Price) as Total "
        << "FROM OrderParts op "
        << "INNER JOIN Parts p ON op.PartID = p.PartID "
        << "WHERE op.OrderID = " << orderID;

    if (!db.ExecuteQuery(ss.str(), hstmt)) return 0.0;

    double total = 0.0;
    SQLFetch(hstmt);
    SQLGetData(hstmt, 1, SQL_C_DOUBLE, &total, sizeof(total), NULL);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return total;
}

// ============ ДОПОЛНИТЕЛЬНАЯ ФУНКЦИЯ 5: Отмена заказа с возвратом товаров ============

bool Order::CancelOrderWithRestore(DatabaseManager& db, int orderID) {
    if (!db.IsConnected()) return false;

    // Проверяем текущий статус
    SQLHSTMT hstmtCheck = NULL;
    std::stringstream checkQuery;
    checkQuery << "SELECT Status FROM Orders WHERE OrderID = " << orderID;

    if (!db.ExecuteQuery(checkQuery.str(), hstmtCheck)) {
        std::cout << "Не удалось проверить статус заказа!" << std::endl;
        return false;
    }

    char status[20];
    SQLRETURN ret = SQLFetch(hstmtCheck);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Заказ не найден!" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtCheck);
        return false;
    }

    SQLGetData(hstmtCheck, 1, SQL_C_CHAR, status, sizeof(status), NULL);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmtCheck);

    std::string statusStr(status);

    if (statusStr == "Cancelled" || statusStr == "Отменен") {
        std::cout << "Заказ уже отменен!" << std::endl;
        return false;
    }

    if (statusStr == "Completed" || statusStr == "Выполнен") {
        std::cout << "Нельзя отменить выполненный заказ!" << std::endl;
        return false;
    }

    // Возвращаем товары на склад
    std::stringstream restoreQuery;
    restoreQuery << "UPDATE p SET p.StockQuantity = p.StockQuantity + op.Quantity "
        << "FROM Parts p "
        << "INNER JOIN OrderParts op ON p.PartID = op.PartID "
        << "WHERE op.OrderID = " << orderID;

    if (!db.ExecuteNonQuery(restoreQuery.str())) {
        std::cout << "Не удалось вернуть товары на склад!" << std::endl;
        return false;
    }

    // Обновляем статус
    std::stringstream updateQuery;
    updateQuery << "UPDATE Orders SET Status = 'Отменен' WHERE OrderID = " << orderID;

    if (!db.ExecuteNonQuery(updateQuery.str())) {
        std::cout << "Не удалось обновить статус заказа!" << std::endl;
        return false;
    }

    std::cout << "Заказ #" << orderID << " отменен, товары возвращены на склад!" << std::endl;
    return true;
}

// ============ СОЗДАНИЕ НОВОГО ЗАКАЗА (ИНТЕРАКТИВНОЕ) ============

void Order::CreateNewOrder(DatabaseManager& db, AuthManager& auth) {
    if (!auth.CanEdit()) {
        std::cout << "Доступ запрещен: Только Менеджер+ может создавать заказы!" << std::endl;
        return;
    }

    std::cout << "\n========== СОЗДАНИЕ НОВОГО ЗАКАЗА ==========" << std::endl;

    // Показываем список доступных деталей
    SQLHSTMT hstmtParts = NULL;
    if (Part::GetAll(db, hstmtParts)) {
        std::cout << "\nДоступные детали:" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        SQLINTEGER pid, pstock;
        char pname[100], pnumber[50];
        double pprice;

        SQLBindCol(hstmtParts, 1, SQL_C_SLONG, &pid, 0, NULL);
        SQLBindCol(hstmtParts, 2, SQL_C_CHAR, pname, sizeof(pname), NULL);
        SQLBindCol(hstmtParts, 3, SQL_C_CHAR, pnumber, sizeof(pnumber), NULL);
        SQLBindCol(hstmtParts, 4, SQL_C_DOUBLE, &pprice, 0, NULL);
        SQLBindCol(hstmtParts, 5, SQL_C_SLONG, &pstock, 0, NULL);

        while (SQLFetch(hstmtParts) == SQL_SUCCESS) {
            std::cout << "ID: " << pid << " | " << pname << " | Цена: " << pprice << " | Остаток: " << pstock << std::endl;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtParts);
    }

    // Выбор деталей для заказа
    std::vector<OrderPartItem> items;
    int currentEmployeeID = auth.GetCurrentUserID();

    std::cout << "\nВведите дату заказа (ГГГГ-ММ-ДД, Enter для сегодня): ";
    std::string orderDate;
    std::cin.ignore();
    std::getline(std::cin, orderDate);
    if (orderDate.empty()) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::stringstream ss;
        ss << 1900 + ltm->tm_year << "-" << (1 + ltm->tm_mon) << "-" << ltm->tm_mday;
        orderDate = ss.str();
    }

    int partCount = 0;
    while (partCount < 3) {
        std::cout << "\n--- Деталь #" << (partCount + 1) << " ---" << std::endl;
        int partId = SafeInputInt("Введите ID детали (0 - завершить): ");

        if (partId == 0) break;

        // Проверяем существует ли деталь
        Part tempPart;
        if (!tempPart.Read(db, partId)) {
            std::cout << "Деталь с ID " << partId << " не найдена!" << std::endl;
            continue;
        }

        int quantity = SafeInputInt("Введите количество: ");
        if (quantity <= 0) {
            std::cout << "Количество должно быть положительным!" << std::endl;
            continue;
        }

        if (quantity > tempPart.GetStockQuantity()) {
            std::cout << "Недостаточно на складе! Доступно: " << tempPart.GetStockQuantity() << std::endl;
            continue;
        }

        OrderPartItem item;
        item.partID = partId;
        item.quantity = quantity;
        items.push_back(item);
        partCount++;

        std::cout << "Деталь добавлена в заказ." << std::endl;
    }

    if (items.empty()) {
        std::cout << "Заказ не создан: не выбрано ни одной детали." << std::endl;
        return;
    }

    // Создаем заказ
    Order order;
    if (order.CreateWithProcedure(db, currentEmployeeID, orderDate, items)) {
        std::cout << "\nЗаказ успешно создан!" << std::endl;
    }
    else {
        std::cout << "\nОшибка при создании заказа!" << std::endl;
    }
}