//This file provides the example usage of the employee api.

#include "employee/employee_api.h"
#include "http_client/http_client.h"
#include <iostream>
#include <memory>

int main() 
{
    
    // Create HTTP client
    auto http_client = std::make_shared<HTTPClient>();
    
    // Create Employee API instance http://localhost:8112/api/v1/employee
    EmployeeApi api(http_client, "http://localhost:8112/api/v1/", 5000);
    
    try 
    {
        std::cout << "=== Creating Employee ===" << std::endl;
        
        Employee new_employee("", "Kaustab Kharche", 75000, 30, "Software Engineer", "");
     
        Employee created_employee;
        auto result = api.createEmployee(new_employee, created_employee);
        
        if (result.success) 
        {
            std::cout << "Employee created successfully!" << std::endl;
            std::cout << "ID: " << created_employee.getId() << std::endl;
            std::cout << "Name: " << created_employee.getName() << std::endl;
            std::cout << "Salary: " << created_employee.getSalary() << std::endl;
            std::cout << "Age: " << created_employee.getAge() << std::endl;
            std::cout << "Title: " << created_employee.getTitle() << std::endl;
            std::cout << "Email: " << created_employee.getEmail() << std::endl;
            
            std::cout << "\n=== Getting Employee by ID ===" << std::endl;
            Employee retrieved_employee;
            auto get_result = api.getEmployeeById(created_employee.getId(), retrieved_employee);
            
            if (get_result.success) 
            {
                std::cout << "Employee retrieved successfully!" << std::endl;
                std::cout << "Name: " << retrieved_employee.getName() << std::endl;
                std::cout << "Salary: " << retrieved_employee.getSalary() << std::endl;
            } 
            else 
            {
                std::cout << "Failed to retrieve employee: " << get_result.message << std::endl;
            }

            std::cout << "\n=== Deleting Employee ===" << std::endl;
            std::string deleted_employee_name;
            auto delete_result = api.deleteEmployeeById(created_employee.getId(), deleted_employee_name);
            
            if (delete_result.success) 
            {
                std::cout << "Employee deleted successfully!" << std::endl;
                std::cout << "Deleted employee name: " << deleted_employee_name << std::endl;
            } 
            else 
            {
                std::cout << "Failed to delete employee: " << delete_result.message << std::endl;
            }
        } 
        else 
        {
            std::cout << "Failed to create employee: " << result.message << std::endl;
        }
        
        std::cout << "\n=== Getting All Employees ===" << std::endl;
        std::vector<Employee> all_employees;
        auto all_result = api.getAllEmployees(all_employees);
        if (all_result.success) 
        {
            if (!all_employees.empty()) 
            {
                std::cout << "Total employees: " << all_employees.size() << std::endl;
                for (const auto& emp : all_employees) 
                {
                    std::cout << emp.getId() << ": " << emp.getName() << std::endl;
                }
            } 
            else 
            {
                std::cout << "No employees found." << std::endl;
            }
        } 
        else 
        {
            std::cerr << "Failed to fetch employees: " << all_result.message << std::endl;
        }
        
        std::cout << "\n=== Searching Employees by Name ===" << std::endl;
        std::vector<Employee> search_results;
        auto search_result = api.getEmployeeByNameSearch("Mi", search_results);
        if (search_result.success) 
        {
            if (!search_results.empty()) 
            {
                std::cout << "Found " << search_results.size() << " employees with 'Mi' their name" << std::endl;
                for (const auto& emp : search_results) 
                {
                    std::cout << emp.getId() << ": " << emp.getName() << std::endl;
                }
            } 
            else 
            {
                std::cout << "No employees found " << std::endl;
            }
        } 
        else 
        {
            std::cerr << "Search failed: " << search_result.message << std::endl;
        }
        
        std::cout << "\n=== Top 10 Highest Earning Employees ===" << std::endl;
        std::vector<std::string> top_earners;
        auto top_result = api.getTop10HighestEarningEmployeeNames(top_earners);
        if (top_result.success) {
            if (!top_earners.empty()) 
            {
                std::cout << "Top " << top_earners.size() << " highest earning employees:" << std::endl;
                for (size_t i = 0; i < top_earners.size(); ++i) 
                {
                    std::cout << (i + 1) << ". " << top_earners[i] << std::endl;
                }
            } 
            else
            {
                std::cout << "No top earners found." << std::endl;
            }
        } 
        else 
        {
            std::cerr << "Failed to fetch top earners: " << top_result.message << std::endl;
        }
        
        std::cout << "\n=== Highest Salary ===" << std::endl;
        int highest_salary;
        auto salary_result = api.getHighestSalaryFromEmployees(highest_salary);
        if (salary_result.success) 
        {
            std::cout << "Highest salary: " << highest_salary << std::endl;
        } 
        else 
        {
            std::cerr << "Failed to fetch highest salary: " << salary_result.message << std::endl;
        }
        
    } 
    catch (const std::invalid_argument& e) 
    {
        std::cerr << "Invalid argument error: " << e.what() << std::endl;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
    }
    
    return 0;
}