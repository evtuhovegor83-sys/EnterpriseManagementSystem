#pragma once
#include <string>
#include <vector>
#include "DatabaseManager.h"

struct OrderPartItem {
    int partID;
    int quantity;
};

class Order {
private:
    int orderID;
    std::string orderDate;
    int employeeID;
    std::string status;
    std::vector<OrderPartItem> items;

public:
    Order();
    Order(int employeeID, const std::string& orderDate);

    // Создание заказа через хранимую процедуру (со списанием остатков)
    bool CreateWithProcedure(DatabaseManager& db, int employeeID, const std::string& orderDate, const std::vector<OrderPartItem>& items);

    // Получение всех заказов с JOIN (сотрудник, детали)
    static bool GetAllOrdersWithDetails(DatabaseManager& db, SQLHSTMT& hstmt);

    // Отчет: Топ-5 деталей по продажам (вызов хранимой процедуры)
    static bool GetTop5PartsBySales(DatabaseManager& db, const std::string& startDate, const std::string& endDate, SQLHSTMT& hstmt);

    // Получение заказов по сотруднику
    static bool GetOrdersByEmployee(DatabaseManager& db, int employeeID, SQLHSTMT& hstmt);

    // Обновление статуса заказа
    bool UpdateStatus(DatabaseManager& db, int orderID, const std::string& newStatus);

    // Getters
    int GetOrderID() const { return orderID; }
    std::string GetOrderDate() const { return orderDate; }
    int GetEmployeeID() const { return employeeID; }
    std::string GetStatus() const { return status; }
};