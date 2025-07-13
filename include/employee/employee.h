#pragma once
#include <string>

class Employee {
public:
    Employee() noexcept;
    Employee(
        std::string employee_id, 
        std::string employee_name, 
        int employee_salary,
        int employee_age,
        std::string employee_title, 
        std::string employee_email);

    Employee(
            std::string employee_name,
            int employee_salary,
            int employee_age,
            std::string employee_title);

    std::string getId() const { return employee_id; }
    std::string getName() const { return employee_name; }
    int getSalary() const { return employee_salary; }
    int getAge() const { return employee_age; }
    std::string getTitle() const { return employee_title; }
    std::string getEmail() const { return employee_email; }

private:
    std::string employee_id;
    std::string employee_name;
    int employee_salary;
    int employee_age; 
    std::string employee_title;
    std::string employee_email;
};