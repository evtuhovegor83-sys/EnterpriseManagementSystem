#include "Part.h"
#include "AuthManager.h"
#include <iostream>
#include <regex>
#include <sstream>

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
        std::cerr << "Error: Invalid part name" << std::endl;
        return false;
    }
    partName = name;
    return true;
}

bool Part::SetPartNumber(const std::string& number) {
    if (number.empty()) {
        std::cerr << "Error: Part number cannot be empty" << std::endl;
        return false;
    }
    partNumber = number;
    return true;
}

bool Part::SetPrice(double price) {
    if (!ValidatePrice(price)) {
        std::cerr << "Error: Price must be between 0 and 10,000,000" << std::endl;
        return false;
    }
    this->price = price;
    return true;
}

bool Part::SetStockQuantity(int quantity) {
    if (!ValidateStockQuantity(quantity)) {
        std::cerr << "Error: Stock quantity cannot be negative" << std::endl;
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

// ÑËÎÆÍÛÉ JOIN çàïðîñ (áîëåå 3 òàáëèö)
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

// ============ ÏÎÑÒÐÀÍÈ×ÍÀß ÍÀÂÈÃÀÖÈß ============

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

// ============ ÄÎÏÎËÍÈÒÅËÜÍÀß ÔÓÍÊÖÈß 3: Îáíîâëåíèå öåíû ñ ïðîâåðêîé ïðàâ ============

bool Part::UpdatePriceWithAuth(DatabaseManager& db, AuthManager& auth, int partID, double newPrice) {
    if (!auth.CanEdit()) {
        std::cout << "Access denied: Only Manager+ can update prices!" << std::endl;
        return false;
    }

    if (!ValidatePrice(newPrice)) {
        std::cerr << "Error: Invalid price value! Price must be between 0 and 10,000,000" << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << "UPDATE Parts SET Price = " << newPrice << " WHERE PartID = " << partID;

    bool result = db.ExecuteNonQuery(ss.str());
    if (result) {
        std::cout << "Price updated successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to update price!" << std::endl;
    }

    return result;
}