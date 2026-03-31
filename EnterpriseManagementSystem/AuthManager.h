#pragma once
#include <string>
#include "DatabaseManager.h"

enum class UserRole {
    ADMIN = 1,
    MANAGER = 2,
    WAREHOUSE = 3,
    ACCOUNTANT = 4
};

class AuthManager {
private:
    int currentUserID;
    std::string currentUserName;
    UserRole currentRole;
    bool authenticated;

public:
    AuthManager();

    // Авторизация по email
    bool Login(DatabaseManager& db, const std::string& email);

    // Выход из системы
    void Logout();

    // Проверка прав доступа
    bool HasPermission(UserRole requiredRole) const;
    bool CanDelete() const;
    bool CanEdit() const;
    bool CanViewReports() const;

    // Getters
    bool IsAuthenticated() const { return authenticated; }
    int GetCurrentUserID() const { return currentUserID; }
    std::string GetCurrentUserName() const { return currentUserName; }
    UserRole GetCurrentRole() const { return currentRole; }

    // Получение названия роли
    static std::string GetRoleName(UserRole role);
};