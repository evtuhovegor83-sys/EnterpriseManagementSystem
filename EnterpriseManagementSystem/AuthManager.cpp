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
        std::cout << "Ошибка входа: Ошибка выполнения запроса" << std::endl;
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
        std::cout << "  ВХОД ВЫПОЛНЕН УСПЕШНО!" << std::endl;
        std::cout << "  Пользователь: " << currentUserName << std::endl;
        std::cout << "  Роль: " << GetRoleName(currentRole) << std::endl;
        std::cout << "========================================" << std::endl;

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return true;
    }

    std::cout << "Ошибка входа: Пользователь с email '" << email << "' не найден" << std::endl;
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return false;
}

void AuthManager::Logout() {
    authenticated = false;
    currentUserID = -1;
    currentUserName = "";
    currentRole = UserRole::WAREHOUSE;
    std::cout << "Выход из системы выполнен успешно" << std::endl;
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
    case UserRole::ADMIN:      return "Администратор";
    case UserRole::MANAGER:    return "Менеджер";
    case UserRole::WAREHOUSE:  return "Складской работник";
    case UserRole::ACCOUNTANT: return "Бухгалтер";
    default:                   return "Неизвестно";
    }
}