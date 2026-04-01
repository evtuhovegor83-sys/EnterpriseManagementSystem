#include "Part.h"
#include "AuthManager.h"
#include <iostream>
#include <regex>
#include <sstream>

// Прототип функции для безопасного ввода (объявлена в main.cpp)
int SafeInputInt(const std::string& prompt);
double SafeInputDouble(const std::string& prompt);

bool Part::ValidatePrice(double price) {
    return price >= 0 && price < 10000000;
}

bool Part::ValidateStockQuantity(int quantity) {
    return quantity >= 0;
}

bool Part::ValidatePartName(const std::string& name) {
    if (name.empty() || name.length() > 100) return false;
    return true;
}

Part::Part() : partID(-1), price(0), stockQuantity(0), warehouseID(-1), supplierID(-1) {}

Part::Part(const std::string& partName, const std::string& partNumber, double price, int stockQuantity)
    : partID(-1), partName(partName), partNumber(partNumber), price(price), stockQuantity(stockQuantity), warehouseID(-1), supplierID(-1) {
}

bool Part::SetPartName(const std::string& name) {
    if (!ValidatePartName(name)) {
        std::cerr << "Ошибка: Неверное название детали" << std::endl;
        return false;
    }
    partName = name;
    return true;
}

bool Part::SetPartNumber(const std::string& number) {
    if (number.empty()) {
        std::cerr << "Ошибка: Артикул детали не может быть пустым" << std::endl;
        return false;
    }
    partNumber = number;
    return true;
}

bool Part::SetPrice(double price) {
    if (!ValidatePrice(price)) {
        std::cerr << "Ошибка: Цена должна быть от 0 до 10 000 000" << std::endl;
        return false;
    }
    this->price = price;
    return true;
}

bool Part::SetStockQuantity(int quantity) {
    if (!ValidateStockQuantity(quantity)) {
        std::cerr << "Ошибка: Количество на складе не может быть отрицательным" << std::endl;
        return false;
    }
    stockQuantity = quantity;
    return true;
}

void Part::SetWarehouseID(int id) {
    warehouseID = id;
}

void Part::SetSupplierID(int id) {
    supplierID = id;
}

bool Part::Create(DatabaseManager& db) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "INSERT INTO Parts (PartName, PartNumber, Price, StockQuantity, WarehouseID, SupplierID) VALUES (";
    ss << "N'" << partName << "', ";
    ss << "N'" << partNumber << "', ";
    ss << price << ", ";
    ss << stockQuantity << ", ";

    if (warehouseID == -1) {
        ss << "NULL, ";
    }
    else {
        ss << warehouseID << ", ";
    }

    if (supplierID == -1) {
        ss << "NULL)";
    }
    else {
        ss << supplierID << ")";
    }

    return db.ExecuteNonQuery(ss.str());
}

bool Part::Read(DatabaseManager& db, int id) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    std::stringstream ss;
    ss << "SELECT PartID, PartName, PartNumber, Price, StockQuantity, WarehouseID, SupplierID FROM Parts WHERE PartID = " << id;

    if (!db.ExecuteQuery(ss.str(), hstmt)) {
        return false;
    }

    SQLRETURN ret = SQLFetch(hstmt);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        partID = id;

        char buffer[256];
        SQLGetData(hstmt, 2, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
        partName = buffer;

        SQLGetData(hstmt, 3, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
        partNumber = buffer;

        SQLGetData(hstmt, 4, SQL_C_DOUBLE, &price, sizeof(price), NULL);

        SQLGetData(hstmt, 5, SQL_C_SLONG, &stockQuantity, sizeof(stockQuantity), NULL);

        SQLGetData(hstmt, 6, SQL_C_SLONG, &warehouseID, sizeof(warehouseID), NULL);

        SQLGetData(hstmt, 7, SQL_C_SLONG, &supplierID, sizeof(supplierID), NULL);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return true;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return false;
}

bool Part::Update(DatabaseManager& db) {
    if (!db.IsConnected() || partID == -1) return false;

    std::stringstream ss;
    ss << "UPDATE Parts SET ";
    ss << "PartName = N'" << partName << "', ";
    ss << "PartNumber = N'" << partNumber << "', ";
    ss << "Price = " << price << ", ";
    ss << "StockQuantity = " << stockQuantity << ", ";
    ss << "WarehouseID = ";

    if (warehouseID == -1) {
        ss << "NULL, ";
    }
    else {
        ss << warehouseID << ", ";
    }

    ss << "SupplierID = ";

    if (supplierID == -1) {
        ss << "NULL ";
    }
    else {
        ss << supplierID << " ";
    }

    ss << "WHERE PartID = " << partID;

    return db.ExecuteNonQuery(ss.str());
}

bool Part::Delete(DatabaseManager& db, int id) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "DELETE FROM Parts WHERE PartID = " << id;
    return db.ExecuteNonQuery(ss.str());
}

bool Part::GetAll(DatabaseManager& db, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;
    return db.ExecuteQuery("SELECT PartID, PartName, PartNumber, Price, StockQuantity FROM Parts", hstmt);
}

bool Part::GetLowStock(DatabaseManager& db, int threshold, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "SELECT PartID, PartName, PartNumber, Price, StockQuantity FROM Parts WHERE StockQuantity < " << threshold;
    return db.ExecuteQuery(ss.str(), hstmt);
}

// СЛОЖНЫЙ JOIN запрос (более 3 таблиц)
bool Part::GetPartsWithDetails(DatabaseManager& db, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    std::string query =
        "SELECT p.PartID, p.PartName, p.PartNumber, p.Price, p.StockQuantity, "
        "w.WarehouseName, w.Location as WarehouseLocation, "
        "s.SupplierName, s.ContactPhone, s.ContactEmail "
        "FROM Parts p "
        "LEFT JOIN Warehouses w ON p.WarehouseID = w.WarehouseID "
        "LEFT JOIN Suppliers s ON p.SupplierID = s.SupplierID "
        "ORDER BY p.PartName";

    return db.ExecuteQuery(query, hstmt);
}

// ============ ПОСТРАНИЧНАЯ НАВИГАЦИЯ ============

bool Part::GetPartsPaginated(DatabaseManager& db, int pageNumber, int pageSize, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    if (pageNumber < 1) pageNumber = 1;
    if (pageSize < 1) pageSize = 10;

    int offset = (pageNumber - 1) * pageSize;

    std::stringstream ss;
    ss << "SELECT PartID, PartName, PartNumber, Price, StockQuantity FROM Parts "
        << "ORDER BY PartID "
        << "OFFSET " << offset << " ROWS "
        << "FETCH NEXT " << pageSize << " ROWS ONLY";

    return db.ExecuteQuery(ss.str(), hstmt);
}

int Part::GetPartsTotalCount(DatabaseManager& db) {
    if (!db.IsConnected()) return 0;

    SQLHSTMT hstmt = NULL;
    if (!db.ExecuteQuery("SELECT COUNT(*) FROM Parts", hstmt)) {
        return 0;
    }

    int count = 0;
    SQLFetch(hstmt);
    SQLGetData(hstmt, 1, SQL_C_SLONG, &count, sizeof(count), NULL);

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return count;
}

// ============ ДОПОЛНИТЕЛЬНАЯ ФУНКЦИЯ 3: Обновление цены с проверкой прав ============

bool Part::UpdatePriceWithAuth(DatabaseManager& db, AuthManager& auth, int partID, double newPrice) {
    if (!auth.CanEdit()) {
        std::cout << "Доступ запрещен: Только Менеджер+ может обновлять цены!" << std::endl;
        return false;
    }

    if (!ValidatePrice(newPrice)) {
        std::cerr << "Ошибка: Неверное значение цены! Цена должна быть от 0 до 10 000 000" << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << "UPDATE Parts SET Price = " << newPrice << " WHERE PartID = " << partID;

    bool result = db.ExecuteNonQuery(ss.str());
    if (result) {
        std::cout << "Цена успешно обновлена!" << std::endl;
    }
    else {
        std::cout << "Не удалось обновить цену!" << std::endl;
    }

    return result;
}

// ============ ДОПОЛНИТЕЛЬНАЯ ФУНКЦИЯ 4: Статистика по складу ============

void Part::GetWarehouseStatistics(DatabaseManager& db) {
    if (!db.IsConnected()) return;

    SQLHSTMT hstmt = NULL;
    std::string query =
        "SELECT w.WarehouseName, "
        "COUNT(p.PartID) as TotalParts, "
        "ISNULL(SUM(p.StockQuantity), 0) as TotalItems, "
        "ISNULL(SUM(p.Price * p.StockQuantity), 0) as TotalValue "
        "FROM Warehouses w "
        "LEFT JOIN Parts p ON w.WarehouseID = p.WarehouseID "
        "GROUP BY w.WarehouseName";

    if (db.ExecuteQuery(query, hstmt)) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "     СТАТИСТИКА ПО СКЛАДАМ" << std::endl;
        std::cout << "========================================" << std::endl;

        char warehouseName[100];
        SQLINTEGER totalParts, totalItems;
        double totalValue;

        SQLBindCol(hstmt, 1, SQL_C_CHAR, warehouseName, sizeof(warehouseName), NULL);
        SQLBindCol(hstmt, 2, SQL_C_SLONG, &totalParts, 0, NULL);
        SQLBindCol(hstmt, 3, SQL_C_SLONG, &totalItems, 0, NULL);
        SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &totalValue, 0, NULL);

        int rowCount = 0;
        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "\nСклад: " << warehouseName << std::endl;
            std::cout << "  ├─ Уникальных деталей: " << totalParts << std::endl;
            std::cout << "  ├─ Всего единиц:      " << totalItems << std::endl;
            std::cout << "  └─ Общая стоимость:   " << totalValue << " RUB" << std::endl;
            rowCount++;
        }

        if (rowCount == 0) {
            std::cout << "Данные по складам не найдены." << std::endl;
        }

        std::cout << "\n========================================" << std::endl;

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    else {
        std::cout << "Не удалось получить статистику по складам!" << std::endl;
    }
}

// ============ ДОБАВЛЕНИЕ НОВОЙ ДЕТАЛИ ============

void Part::AddNewPart(DatabaseManager& db, AuthManager& auth) {
    if (!auth.CanEdit()) {
        std::cout << "Доступ запрещен: Только Менеджер+ может добавлять детали!" << std::endl;
        return;
    }

    std::cout << "\n========== ДОБАВЛЕНИЕ НОВОЙ ДЕТАЛИ ==========" << std::endl;

    Part newPart;

    std::cin.ignore();
    std::string name;
    std::cout << "Введите название детали: ";
    std::getline(std::cin, name);
    if (!newPart.SetPartName(name)) return;

    std::string number;
    std::cout << "Введите артикул: ";
    std::getline(std::cin, number);
    if (!newPart.SetPartNumber(number)) return;

    double price = SafeInputDouble("Введите цену: ");
    if (!newPart.SetPrice(price)) return;

    int stock = SafeInputInt("Введите количество на складе: ");
    if (!newPart.SetStockQuantity(stock)) return;

    // Показываем доступные склады
    SQLHSTMT hstmtWarehouse = NULL;
    std::string warehouseQuery = "SELECT WarehouseID, WarehouseName FROM Warehouses";
    if (db.ExecuteQuery(warehouseQuery, hstmtWarehouse)) {
        std::cout << "\nДоступные склады:" << std::endl;
        SQLINTEGER wid;
        char wname[100];
        SQLBindCol(hstmtWarehouse, 1, SQL_C_SLONG, &wid, 0, NULL);
        SQLBindCol(hstmtWarehouse, 2, SQL_C_CHAR, wname, sizeof(wname), NULL);
        while (SQLFetch(hstmtWarehouse) == SQL_SUCCESS) {
            std::cout << "ID: " << wid << " | " << wname << std::endl;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtWarehouse);
    }
    int warehouseId = SafeInputInt("Введите ID склада (0 - без склада): ");
    if (warehouseId != 0) newPart.SetWarehouseID(warehouseId);

    // Показываем доступных поставщиков!!
    SQLHSTMT hstmtSupplier = NULL;
    std::string supplierQuery = "SELECT SupplierID, SupplierName FROM Suppliers";
    if (db.ExecuteQuery(supplierQuery, hstmtSupplier)) {
        std::cout << "\nДоступные поставщики:" << std::endl;
        SQLINTEGER sid;
        char sname[100];
        SQLBindCol(hstmtSupplier, 1, SQL_C_SLONG, &sid, 0, NULL);
        SQLBindCol(hstmtSupplier, 2, SQL_C_CHAR, sname, sizeof(sname), NULL);
        while (SQLFetch(hstmtSupplier) == SQL_SUCCESS) {
            std::cout << "ID: " << sid << " | " << sname << std::endl;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtSupplier);
    }
    int supplierId = SafeInputInt("Введите ID поставщика (0 - без поставщика): ");
    if (supplierId != 0) newPart.SetSupplierID(supplierId);

    if (newPart.Create(db)) {
        std::cout << "\nДеталь успешно добавлена!" << std::endl;
    }
    else {
        std::cout << "\nОшибка при добавлении детали!" << std::endl;
    }
}