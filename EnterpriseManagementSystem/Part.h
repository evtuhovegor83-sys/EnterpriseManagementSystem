#pragma once
#include <string>
#include "DatabaseManager.h"

class Part {
private:
    int partID;
    std::string partName;
    std::string partNumber;
    double price;
    int stockQuantity;
    int warehouseID;
    int supplierID;

    bool ValidatePrice(double price);
    bool ValidateStockQuantity(int quantity);
    bool ValidatePartName(const std::string& name);

public:
    Part();
    Part(const std::string& partName, const std::string& partNumber, double price, int stockQuantity);

    // CRUD операции
    bool Create(DatabaseManager& db);
    bool Read(DatabaseManager& db, int id);
    bool Update(DatabaseManager& db);
    bool Delete(DatabaseManager& db, int id);

    // Получение всех деталей
    static bool GetAll(DatabaseManager& db, SQLHSTMT& hstmt);

    // Получение деталей с остатком меньше указанного
    static bool GetLowStock(DatabaseManager& db, int threshold, SQLHSTMT& hstmt);

    // Сложный JOIN запрос: детали + склад + поставщик
    static bool GetPartsWithDetails(DatabaseManager& db, SQLHSTMT& hstmt);

    // ============ ПОСТРАНИЧНАЯ НАВИГАЦИЯ ============
    // Получение деталей с пагинацией (OFFSET/FETCH)
    static bool GetPartsPaginated(DatabaseManager& db, int pageNumber, int pageSize, SQLHSTMT& hstmt);

    // Получение общего количества деталей
    static int GetPartsTotalCount(DatabaseManager& db);

    // Getters
    int GetID() const { return partID; }
    std::string GetPartName() const { return partName; }
    std::string GetPartNumber() const { return partNumber; }
    double GetPrice() const { return price; }
    int GetStockQuantity() const { return stockQuantity; }
    int GetWarehouseID() const { return warehouseID; }
    int GetSupplierID() const { return supplierID; }

    // Setters
    bool SetPartName(const std::string& name);
    bool SetPartNumber(const std::string& number);
    bool SetPrice(double price);
    bool SetStockQuantity(int quantity);
    void SetWarehouseID(int id);
    void SetSupplierID(int id);
};