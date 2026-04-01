#include "Employee.h"
#include "AuthManager.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <ctime>

// Прототипы функций для безопасного ввода (объявлены в main.cpp)
int SafeInputInt(const std::string& prompt);
double SafeInputDouble(const std::string& prompt);

bool Employee::ValidateEmail(const std::string& email) {
    std::regex pattern(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    return std::regex_match(email, pattern);
}

bool Employee::ValidateSalary(double salary) {
    return salary > 0 && salary < 10000000;
}

bool Employee::ValidateName(const std::string& name) {
    if (name.empty() || name.length() > 50) return false;
    std::regex pattern(R"(^[A-Za-zА-Яа-яёЁ-]{2,50}$)");
    return std::regex_match(name, pattern);
}

Employee::Employee() : employeeID(-1), salary(0), departmentID(-1) {}

Employee::Employee(const std::string& lastName, const std::string& firstName, const std::string& email, double salary)
    : employeeID(-1), lastName(lastName), firstName(firstName), email(email), salary(salary), departmentID(-1) {
}

bool Employee::SetLastName(const std::string& name) {
    if (!ValidateName(name)) {
        std::cerr << "Ошибка: Неверная фамилия. Используйте только буквы, длина 2-50 символов." << std::endl;
        return false;
    }
    lastName = name;
    return true;
}

bool Employee::SetFirstName(const std::string& name) {
    if (!ValidateName(name)) {
        std::cerr << "Ошибка: Неверное имя. Используйте только буквы, длина 2-50 символов." << std::endl;
        return false;
    }
    firstName = name;
    return true;
}

bool Employee::SetMiddleName(const std::string& name) {
    if (!name.empty() && !ValidateName(name)) {
        std::cerr << "Ошибка: Неверное отчество. Используйте только буквы, длина 2-50 символов." << std::endl;
        return false;
    }
    middleName = name;
    return true;
}

bool Employee::SetEmail(const std::string& email) {
    if (!ValidateEmail(email)) {
        std::cerr << "Ошибка: Неверный формат email. Пример: name@company.ru" << std::endl;
        return false;
    }
    this->email = email;
    return true;
}

void Employee::SetHireDate(const std::string& date) {
    hireDate = date;
}

bool Employee::SetSalary(double salary) {
    if (!ValidateSalary(salary)) {
        std::cerr << "Ошибка: Зарплата должна быть положительным числом не более 10 000 000." << std::endl;
        return false;
    }
    this->salary = salary;
    return true;
}

void Employee::SetDepartmentID(int id) {
    departmentID = id;
}

bool Employee::Create(DatabaseManager& db) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;

    ss << "INSERT INTO Employees (LastName, FirstName, MiddleName, Email, HireDate, Salary, DepartmentID) VALUES (";
    ss << "N'" << lastName << "', ";
    ss << "N'" << firstName << "', ";

    if (middleName.empty()) {
        ss << "NULL, ";
    }
    else {
        ss << "N'" << middleName << "', ";
    }

    ss << "N'" << email << "', ";
    ss << "GETDATE(), ";
    ss << salary << ", ";

    if (departmentID == -1) {
        ss << "NULL)";
    }
    else {
        ss << departmentID << ")";
    }

    return db.ExecuteNonQuery(ss.str());
}

bool Employee::Read(DatabaseManager& db, int id) {
    if (!db.IsConnected()) return false;

    SQLHSTMT hstmt = NULL;
    std::stringstream ss;
    ss << "SELECT EmployeeID, LastName, FirstName, MiddleName, Email, CONVERT(DATE, HireDate) AS HireDate, Salary, DepartmentID FROM Employees WHERE EmployeeID = " << id;

    if (!db.ExecuteQuery(ss.str(), hstmt)) {
        return false;
    }

    SQLRETURN ret = SQLFetch(hstmt);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        employeeID = id;

        char buffer[256];
        SQLGetData(hstmt, 2, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
        lastName = buffer;

        SQLGetData(hstmt, 3, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
        firstName = buffer;

        SQLGetData(hstmt, 4, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
        middleName = buffer;

        SQLGetData(hstmt, 5, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
        email = buffer;

        char dateBuffer[20];
        SQLGetData(hstmt, 6, SQL_C_CHAR, dateBuffer, sizeof(dateBuffer), NULL);
        hireDate = dateBuffer;

        SQLGetData(hstmt, 7, SQL_C_DOUBLE, &salary, sizeof(salary), NULL);

        SQLGetData(hstmt, 8, SQL_C_SLONG, &departmentID, sizeof(departmentID), NULL);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return true;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return false;
}

bool Employee::Update(DatabaseManager& db) {
    if (!db.IsConnected() || employeeID == -1) return false;

    std::stringstream ss;
    ss << "UPDATE Employees SET ";
    ss << "LastName = N'" << lastName << "', ";
    ss << "FirstName = N'" << firstName << "', ";
    ss << "MiddleName = ";

    if (middleName.empty()) {
        ss << "NULL, ";
    }
    else {
        ss << "N'" << middleName << "', ";
    }

    ss << "Email = N'" << email << "', ";
    ss << "Salary = " << salary << ", ";
    ss << "DepartmentID = ";

    if (departmentID == -1) {
        ss << "NULL ";
    }
    else {
        ss << departmentID << " ";
    }

    ss << "WHERE EmployeeID = " << employeeID;

    return db.ExecuteNonQuery(ss.str());
}

bool Employee::Delete(DatabaseManager& db, int id) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "DELETE FROM Employees WHERE EmployeeID = " << id;
    return db.ExecuteNonQuery(ss.str());
}

bool Employee::DeleteWithAuth(DatabaseManager& db, AuthManager& auth, int id) {
    if (!auth.CanDelete()) {
        std::cout << "Доступ запрещен: Только Администратор может удалять сотрудников!" << std::endl;
        return false;
    }
    return Delete(db, id);
}

bool Employee::GetAll(DatabaseManager& db, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;
    return db.ExecuteQuery("SELECT EmployeeID, LastName, FirstName, Email, Salary FROM Employees", hstmt);
}

bool Employee::Search(DatabaseManager& db, const std::string& departmentName, int minSalary, int maxSalary, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "SELECT e.EmployeeID, e.LastName, e.FirstName, e.Email, e.Salary, d.DepartmentName "
        << "FROM Employees e "
        << "LEFT JOIN Departments d ON e.DepartmentID = d.DepartmentID "
        << "WHERE 1=1";

    if (!departmentName.empty()) {
        ss << " AND d.DepartmentName = N'" << departmentName << "'";
    }
    if (minSalary > 0) {
        ss << " AND e.Salary >= " << minSalary;
    }
    if (maxSalary > 0) {
        ss << " AND e.Salary <= " << maxSalary;
    }

    return db.ExecuteQuery(ss.str(), hstmt);
}

bool Employee::GetEmployeesByDepartment(DatabaseManager& db, int departmentID, SQLHSTMT& hstmt) {
    if (!db.IsConnected()) return false;

    std::stringstream ss;
    ss << "SELECT EmployeeID, LastName, FirstName, Email, Salary "
        << "FROM Employees WHERE DepartmentID = " << departmentID;

    return db.ExecuteQuery(ss.str(), hstmt);
}

// ============ ДОБАВЛЕНИЕ НОВОГО СОТРУДНИКА ============

void Employee::AddNewEmployee(DatabaseManager& db, AuthManager& auth) {
    if (!auth.CanDelete()) {
        std::cout << "Доступ запрещен: Только Администратор может добавлять сотрудников!" << std::endl;
        return;
    }

    std::cout << "\n========== ДОБАВЛЕНИЕ НОВОГО СОТРУДНИКА ==========" << std::endl;

    Employee newEmp;

    std::cin.ignore();
    std::string lastName;
    std::cout << "Введите фамилию: ";
    std::getline(std::cin, lastName);
    if (!newEmp.SetLastName(lastName)) return;

    std::string firstName;
    std::cout << "Введите имя: ";
    std::getline(std::cin, firstName);
    if (!newEmp.SetFirstName(firstName)) return;

    std::string middleName;
    std::cout << "Введите отчество (Enter - пропустить): ";
    std::getline(std::cin, middleName);
    if (!middleName.empty()) newEmp.SetMiddleName(middleName);

    std::string email;
    std::cout << "Введите email: ";
    std::getline(std::cin, email);
    if (!newEmp.SetEmail(email)) return;

    double salary = SafeInputDouble("Введите зарплату: ");
    if (!newEmp.SetSalary(salary)) return;

    // Показываем доступные отделы
    SQLHSTMT hstmtDept = NULL;
    std::string deptQuery = "SELECT DepartmentID, DepartmentName FROM Departments";
    if (db.ExecuteQuery(deptQuery, hstmtDept)) {
        std::cout << "\nДоступные отделы:" << std::endl;
        SQLINTEGER did;
        char dname[100];
        SQLBindCol(hstmtDept, 1, SQL_C_SLONG, &did, 0, NULL);
        SQLBindCol(hstmtDept, 2, SQL_C_CHAR, dname, sizeof(dname), NULL);
        while (SQLFetch(hstmtDept) == SQL_SUCCESS) {
            std::cout << "ID: " << did << " | " << dname << std::endl;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtDept);
    }
    int deptId = SafeInputInt("Введите ID отдела (0 - без отдела): ");
    if (deptId != 0) newEmp.SetDepartmentID(deptId);

    // Показываем доступные роли
    SQLHSTMT hstmtRole = NULL;
    std::string roleQuery = "SELECT RoleID, RoleName FROM Roles";
    if (db.ExecuteQuery(roleQuery, hstmtRole)) {
        std::cout << "\nДоступные роли:" << std::endl;
        SQLINTEGER rid;
        char rname[100];
        SQLBindCol(hstmtRole, 1, SQL_C_SLONG, &rid, 0, NULL);
        SQLBindCol(hstmtRole, 2, SQL_C_CHAR, rname, sizeof(rname), NULL);
        while (SQLFetch(hstmtRole) == SQL_SUCCESS) {
            std::cout << "ID: " << rid << " | " << rname << std::endl;
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmtRole);
    }
    int roleId = SafeInputInt("Введите ID роли: ");

    // Устанавливаем дату приема сегодняшним числом (исправлено: localtime_s)
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    std::stringstream hireDate;
    hireDate << 1900 + ltm.tm_year << "-" << (1 + ltm.tm_mon) << "-" << ltm.tm_mday;
    newEmp.SetHireDate(hireDate.str());

    if (newEmp.Create(db)) {
        // Получаем ID нового сотрудника
        SQLHSTMT hstmtLast = NULL;
        if (db.ExecuteQuery("SELECT MAX(EmployeeID) FROM Employees", hstmtLast)) {
            int newId = 0;
            SQLFetch(hstmtLast);
            SQLGetData(hstmtLast, 1, SQL_C_SLONG, &newId, sizeof(newId), NULL);
            SQLFreeHandle(SQL_HANDLE_STMT, hstmtLast);

            if (newId > 0) {
                std::stringstream roleInsert;
                roleInsert << "INSERT INTO UserRoles (EmployeeID, RoleID) VALUES (" << newId << ", " << roleId << ")";
                if (db.ExecuteNonQuery(roleInsert.str())) {
                    std::cout << "\nСотрудник успешно добавлен!" << std::endl;
                    std::cout << "ID нового сотрудника: " << newId << std::endl;
                }
                else {
                    std::cout << "\nСотрудник добавлен, но не удалось назначить роль!" << std::endl;
                }
            }
            else {
                std::cout << "\nСотрудник добавлен!" << std::endl;
            }
        }
        else {
            std::cout << "\nСотрудник добавлен!" << std::endl;
        }
    }
    else {
        std::cout << "\nОшибка при добавлении сотрудника!" << std::endl;
    }
}