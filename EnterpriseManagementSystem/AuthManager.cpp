#include "AuthManager.h"
#include <iostream>
#include <sstream>

AuthManager::AuthManager() : currentUserID(-1), currentUserName(""), currentRole(UserRole::WAREHOUSE), authenticated(false) {}

bool AuthManager::Login(DatabaseManager& db, const std::string& email) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    std::stringstream ss;
    ss << "SELECT e.EmployeeID, e.LastName + ' ' + e.FirstName AS FullName, r.RoleID "
        << "FROM Employees e "
        << "INNER JOIN UserRoles ur ON e.EmployeeID = ur.EmployeeID "
        << "INNER JOIN Roles r ON ur.RoleID = r.RoleID "
        << "WHERE e.Email = '" << email << "'";

    if (!db.ExecuteQuery(ss.str(), hstmt)) {
        std::cout << "Login failed: Query error" << std::endl;
        return false;
    }

    SQLRETURN ret = SQLFetch(hstmt);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &currentUserID, sizeof(currentUserID), NULL);

        char nameBuffer[256];
        SQLGetData(hstmt, 2, SQL_C_CHAR, nameBuffer, sizeof(nameBuffer), NULL);
        currentUserName = nameBuffer;

        int roleID;
        SQLGetData(hstmt, 3, SQL_C_SLONG, &roleID, sizeof(roleID), NULL);
        currentRole = static_cast<UserRole>(roleID);

        authenticated = true;

        std::cout << "\n========================================" << std::endl;
        std::cout << "  LOGIN SUCCESSFUL!" << std::endl;
        std::cout << "  User: " << currentUserName << std::endl;
        std::cout << "  Role: " << GetRoleName(currentRole) << std::endl;
        std::cout << "========================================" << std::endl;

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return true;
    }

    std::cout << "Login failed: User with email '" << email << "' not found" << std::endl;
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return false;
}

void AuthManager::Logout() {
    authenticated = false;
    currentUserID = -1;
    currentUserName = "";
    currentRole = UserRole::WAREHOUSE;
    std::cout << "Logged out successfully" << std::endl;
}

bool AuthManager::HasPermission(UserRole requiredRole) const {
    if (!authenticated) return false;

    // Администратор имеет все права
    if (currentRole == UserRole::ADMIN) return true;

    // Проверка конкретной роли
    return currentRole == requiredRole;
}

bool AuthManager::CanDelete() const {
    if (!authenticated) return false;
    // Только администратор может удалять
    return currentRole == UserRole::ADMIN;
}

bool AuthManager::CanEdit() const {
    if (!authenticated) return false;
    // Администратор и менеджер могут редактировать
    return (currentRole == UserRole::ADMIN || currentRole == UserRole::MANAGER);
}

bool AuthManager::CanViewReports() const {
    if (!authenticated) return false;
    // Все кроме складского работника могут смотреть отчеты
    return (currentRole != UserRole::WAREHOUSE);
}

std::string AuthManager::GetRoleName(UserRole role) {
    switch (role) {
    case UserRole::ADMIN:      return "Administrator";
    case UserRole::MANAGER:    return "Manager";
    case UserRole::WAREHOUSE:  return "Warehouse Worker";
    case UserRole::ACCOUNTANT: return "Accountant";
    default:                   return "Unknown";
    }
}