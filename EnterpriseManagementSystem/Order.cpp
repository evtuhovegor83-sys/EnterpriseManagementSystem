#include "Order.h"
#include <iostream>
#include <sstream>

Order::Order() : orderID(-1), employeeID(-1), status("") {}

Order::Order(int employeeID, const std::string& orderDate)
    : orderID(-1), employeeID(employeeID), orderDate(orderDate), status("New") {
}

bool Order::CreateWithProcedure(DatabaseManager& db, int employeeID, const std::string& orderDate, const std::vector<OrderPartItem>& items) {
    if (!db.IsConnected()) return false;

    // Ограничиваем до 3 деталей (как в нашей хранимой процедуре)
    if (items.empty() || items.size() > 3) {
        std::cerr << "Error: Order must contain between 1 and 3 parts" << std::endl;
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

    std::cout << "Executing: " << ss.str() << std::endl;

    SQLHSTMT hstmt = NULL;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, db.GetConnection(), &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    std::wstring wQuery(ss.str().begin(), ss.str().end());
    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        // Получаем результат
        SQLINTEGER resultOrderID;
        char message[256];

        SQLBindCol(hstmt, 1, SQL_C_SLONG, &resultOrderID, 0, NULL);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, message, sizeof(message), NULL);

        if (SQLFetch(hstmt) == SQL_SUCCESS) {
            orderID = resultOrderID;
            std::cout << "Result: " << message << " (OrderID=" << orderID << ")" << std::endl;
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

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, db.GetConnection(), &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        return false;
    }

    std::wstring wQuery(ss.str().begin(), ss.str().end());
    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);

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