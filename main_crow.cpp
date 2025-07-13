#include "employee/employee_api.h"
#include "http_client/http_client.h"
#include "employee/employee_json.h"
#include <crow.h>
#include <nlohmann/json.hpp>
#include <memory>

int main() {
    std::shared_ptr<IHttpClient> httpClient = std::make_shared<HTTPClient>();
    auto employeeApi = std::make_shared<EmployeeApi>(
        httpClient, 
        "http://localhost:8112/api/v1/", 
        5000
    );

    crow::SimpleApp app;

    // Route: GET /api/v1/employees/highestsalary
    CROW_ROUTE(app, "/api/v1/employees/highestsalary").methods(crow::HTTPMethod::Get)
    ([employeeApi](const crow::request& req) 
    {
        try 
        {
            int highest_salary = -1;
            auto salary_result = employeeApi->getHighestSalaryFromEmployees(highest_salary);
            if (salary_result.success) 
            {
                nlohmann::json json = { {"highest_salary", highest_salary} };
                return crow::response(200, json.dump());
            } 
            else 
            {
                return crow::response(salary_result.status_code, "{\"error\": \"" + salary_result.message + "\"}");
            }
        } 
        catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in getHighestSalaryFromEmployees: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Route: GET /api/v1/employees/top-10-highest-earning
    CROW_ROUTE(app, "/api/v1/employees/top-10-highest-earning").methods(crow::HTTPMethod::Get)
    ([employeeApi](const crow::request& req) 
    {
        try 
        {
            std::vector<std::string> names;
            auto names_result = employeeApi->getTop10HighestEarningEmployeeNames(names);
            if (names_result.success) 
            {
                nlohmann::json json = { {"employee_names", names} };
                return crow::response(200, json.dump());
            } 
            else 
            {
                return crow::response(names_result.status_code, "{\"error\": \"" + names_result.message + "\"}");
            }
        } 
        catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in getTop10HighestEarningEmployeeNames: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Route: GET /api/v1/employees/search/<string>
    CROW_ROUTE(app, "/api/v1/employees/search/<string>").methods(crow::HTTPMethod::Get)
    ([employeeApi](const crow::request& req, const std::string& name) 
    {
        try 
        {
            //std::string name = req.url_params.get("name");
            if (name.empty()) 
            {
                return crow::response(400, "Name parameter is required");
            }
            std::vector<Employee> employees;
            auto employees_result = employeeApi->getEmployeeByNameSearch(name, employees);
            if (employees_result.success) 
            {
                nlohmann::json json = employees;
                return crow::response(200, json.dump());
            } 
            else 
            {
                return crow::response(employees_result.status_code, "{\"error\": \"" + employees_result.message + "\"}");
            }
        } catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in getEmployeeByNameSearch: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Route: GET /api/v1/employees/<string>
    CROW_ROUTE(app, "/api/v1/employees/<string>").methods(crow::HTTPMethod::Get)
    ([employeeApi](const crow::request& req, const std::string& id) 
    {
        try 
        {
            Employee employee;
            auto result = employeeApi->getEmployeeById(id, employee);
            if (result.success) 
            {
                nlohmann::json json = employee;
                return crow::response(200, json.dump());
            } 
            else 
            {
                return crow::response(result.status_code, "{\"error\": \"" + result.message + "\"}");
            }
        } catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in getEmployeeById: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Route: POST /api/v1/employees
    CROW_ROUTE(app, "/api/v1/employees").methods(crow::HTTPMethod::Post)
    ([employeeApi](const crow::request& req) 
    {
        try 
        {
            auto json = nlohmann::json::parse(req.body);
            Employee employee(
                json["name"],
                json["salary"],
                json["age"],
                json["title"]
            );

            Employee created_employee;
            auto result = employeeApi->createEmployee(employee, created_employee);
            if (result.success) 
            {
                nlohmann::json response = {{"message", "Employee created successfully"}, {"employee", created_employee}};
                return crow::response(201, response.dump());
            } 
            else 
            {
                return crow::response(result.status_code, "{\"error\": \"" + result.message + "\"}");
            }
        } catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in createEmployee: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Route: DELETE /api/v1/employees/<string>
    CROW_ROUTE(app, "/api/v1/employees/<string>").methods(crow::HTTPMethod::Delete)
    ([employeeApi](const crow::request& req, const std::string& id) 
    {
        try 
        {
            std::string employee_name;
            auto result = employeeApi->deleteEmployeeById(id, employee_name);
            if (result.success)
            {
                nlohmann::json response = {{"message", "Employee deleted successfully"}, {"deleted_employee_name", employee_name}};
                return crow::response(200, response.dump());
            } 
            else 
            {
                return crow::response(result.status_code, "{\"error\": \"" + result.message + "\"}");
            }
        } 
        catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in deleteEmployee: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Route: GET /api/v1/employees
    CROW_ROUTE(app, "/api/v1/employees").methods(crow::HTTPMethod::Get)
    ([employeeApi](const crow::request& req) 
    {
        try 
        {
            std::vector<Employee> employees;
            auto employees_result = employeeApi->getAllEmployees(employees);
            if (employees_result.success) 
            {
                nlohmann::json json = employees;
                return crow::response(200, json.dump());
            } 
            else 
            {
                return crow::response(employees_result.status_code, "{\"error\": \"" + employees_result.message + "\"}");
            }
        } 
        catch (const std::exception& e) 
        {
            CROW_LOG_ERROR << "Error in getAllEmployees: " << e.what();
            return crow::response(500, e.what());
        }
    });

    // Configure and run the server
    std::cout << "Starting Crow server on port 8111" << std::endl;
    app.port(8111).multithreaded().run();
    return 0;
}