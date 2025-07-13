#include "employee/employee_json.h"

void to_json(nlohmann::json& j, const Employee& employee) {
    j = nlohmann::json{
        {"id", employee.getId()},
        {"employee_name", employee.getName()},
        {"employee_salary", employee.getSalary()},
        {"employee_age", employee.getAge()},
        {"employee_title", employee.getTitle()},
        {"employee_email", employee.getEmail()}
    };
}

void from_json(const nlohmann::json& j, Employee& employee) {
    employee = Employee(
        j.value("id", ""),
        j.value("employee_name", ""),
        j.value("employee_salary", 0),
        j.value("employee_age", 0),
        j.value("employee_title", ""),
        j.value("employee_email", "")
    );
}

void to_json_create_employee(nlohmann::json& j, const Employee& employee) {
    j = nlohmann::json{
        {"name", employee.getName()},
        {"salary", employee.getSalary()},
        {"age", employee.getAge()},
        {"title", employee.getTitle()}   
    };
}