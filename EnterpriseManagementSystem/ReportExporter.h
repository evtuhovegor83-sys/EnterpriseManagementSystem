#pragma once
#include <string>
#include "DatabaseManager.h"

class ReportExporter {
public:
    static bool ExportToCSV(SQLHSTMT hstmt, const std::string& filename, const std::string& headers);
    static bool ExportEmployeesToCSV(DatabaseManager& db, const std::string& filename);
    static bool ExportPartsToCSV(DatabaseManager& db, const std::string& filename);
    static bool ExportOrdersReportToCSV(DatabaseManager& db, const std::string& filename);
    static bool ExportTop5PartsToCSV(DatabaseManager& db, const std::string& startDate, const std::string& endDate, const std::string& filename);
    static bool ExportLowStockToCSV(DatabaseManager& db, int threshold, const std::string& filename);
};