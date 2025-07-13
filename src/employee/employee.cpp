#include "employee/employee.h"

Employee::Employee() noexcept
    : employee_id(""),
      employee_name(""),
      employee_salary(0),
      employee_age(0),
      employee_title(""),
      employee_email("") {}

Employee::Employee(
    std::string employee_id,
    std::string employee_name,
    int employee_salary,
    int employee_age,
    std::string employee_title,
    std::string employee_email)
    : employee_id(std::move(employee_id)),
      employee_name(std::move(employee_name)),
      employee_salary(employee_salary),
      employee_age(employee_age),
      employee_title(std::move(employee_title)),
      employee_email(std::move(employee_email)) {}

Employee::Employee(
        std::string employee_name,
        int employee_salary,
        int employee_age,
        std::string employee_title)
        : employee_id(""),
          employee_name(std::move(employee_name)),
          employee_salary(employee_salary),
          employee_age(employee_age),
          employee_title(std::move(employee_title)),
          employee_email("") {}