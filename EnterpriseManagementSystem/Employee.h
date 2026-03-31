#pragma once
#include <string>
#include "DatabaseManager.h"

// Forward declaration для избежания циклической зависимости
class AuthManager;

class Employee {
private:
    int employeeID;
    std::string lastName;
    std::string firstName;
    std::string middleName;
    std::string email;
    std::string hireDate;
    double salary;
    int departmentID;

    bool ValidateEmail(const std::string& email);
    bool ValidateSalary(double salary);
    bool ValidateName(const std::string& name);

public:
    Employee();
    Employee(const std::string& lastName, const std::string& firstName, const std::string& email, double salary);

    // CRUD операции
    bool Create(DatabaseManager& db);
    bool Read(DatabaseManager& db, int id);
    bool Update(DatabaseManager& db);
    bool Delete(DatabaseManager& db, int id);

    // Удаление с проверкой прав (требует AuthManager)
    bool DeleteWithAuth(DatabaseManager& db, AuthManager& auth, int id);

    // Статические методы
    static bool GetAll(DatabaseManager& db, SQLHSTMT& hstmt);
    static bool Search(DatabaseManager& db, const std::string& departmentName, int minSalary, int maxSalary, SQLHSTMT& hstmt);

    // Getters
    int GetID() const { return employeeID; }
    std::string GetLastName() const { return lastName; }
    std::string GetFirstName() const { return firstName; }
    std::string GetMiddleName() const { return middleName; }
    std::string GetEmail() const { return email; }
    std::string GetHireDate() const { return hireDate; }
    double GetSalary() const { return salary; }
    int GetDepartmentID() const { return departmentID; }

    // Setters
    bool SetLastName(const std::string& name);
    bool SetFirstName(const std::string& name);
    bool SetMiddleName(const std::string& name);
    bool SetEmail(const std::string& email);
    void SetHireDate(const std::string& date);
    bool SetSalary(double salary);
    void SetDepartmentID(int id);

    // Дополнительная функция 2: Получение сотрудников по отделу
    static bool GetEmployeesByDepartment(DatabaseManager& db, int departmentID, SQLHSTMT& hstmt);
};