#include <catch2/catch_test_macros.hpp>
#include "employee/employee_api.h"
#include "http_client/http_client.h"
#include <memory>
#include <nlohmann/json.hpp>

// Mock HTTP client for testing EmployeeApi
class MockEmployeeHttpClient : public IHttpClient 
{
public:
    MockEmployeeHttpClient() : should_fail_(false), status_code_(200), call_count_(0) {}
    
    void set_should_fail(bool should_fail) 
    { 
        should_fail_ = should_fail; 
    }
    void set_status_code(int status_code) 
    { 
        status_code_ = status_code; 
    }
    void set_response_body(const std::string& body) 
    { 
        response_body_ = body; 
    }
    void set_delete_response_body(const std::string& body)
    {
        delete_response_body_ = body;
    }
    
    HTTPResponse request(const HTTPRequest& request) const override 
    {
        call_count_++;
        
        if (should_fail_) 
        {
            return HTTPResponse(0, "", {}, "Mock error", error_type_);
        }
        
        // For DELETE requests, return the delete response body
        if (request.get_method() == HttpMethod::DELETE) {
            return HTTPResponse(status_code_, delete_response_body_, {}, "", HttpErrorType::NONE);
        }
        
        return HTTPResponse(status_code_, response_body_, {}, "", HttpErrorType::NONE);
    }
    
    int get_call_count() const { return call_count_; }
    
private:
    bool should_fail_;
    int status_code_;
    std::string response_body_;
    std::string delete_response_body_ = R"({"status": "success", "data": true})";
    mutable int call_count_;
    HttpErrorType error_type_ = HttpErrorType::NETWORK_ERROR;
};

TEST_CASE("EmployeeApi initialization", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    
    SECTION("Valid initialization") 
    {
        EmployeeApi api(mock_client, "https://api.example.com");
        REQUIRE_NOTHROW(api);
    }
    
    SECTION("Empty base URL") 
    {
        EmployeeApi api(mock_client, "");
        REQUIRE_NOTHROW(api);
    }
}

TEST_CASE("EmployeeApi createEmployee", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    Employee input("", "Mahesh Dekhle", 75000, 30, "Software Engineer", "md@example.com");
    
    SECTION("Successful creation") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": {
                "id": "123",
                "employee_name": "Mahesh Dekhle",
                "employee_salary": 75000,
                "employee_age": 30,
                "employee_title": "Software Engineer",
                "employee_email": "md@example.com"
            }
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(201);
        
        Employee created_employee;
        auto result = api.createEmployee(input, created_employee);
        
        REQUIRE(result.success);
        REQUIRE(created_employee.getId() == "123");
        REQUIRE(created_employee.getName() == "Mahesh Dekhle");
        REQUIRE(created_employee.getSalary() == 75000);
        REQUIRE(created_employee.getAge() == 30);
        REQUIRE(created_employee.getTitle() == "Software Engineer");
        REQUIRE(created_employee.getEmail() == "md@example.com");
    }
    
    SECTION("HTTP error should return failure") 
    {
        mock_client->set_should_fail(true);
        
        Employee created_employee;
        auto result = api.createEmployee(input, created_employee);
        
        REQUIRE_FALSE(result.success);
    }
    
    SECTION("Invalid JSON response should return failure") 
    {
        mock_client->set_response_body("invalid json");
        mock_client->set_status_code(201);
        
        Employee created_employee;
        auto result = api.createEmployee(input, created_employee);
        
        REQUIRE_FALSE(result.success);
    }
    
    SECTION("Non-success status code should return failure") {
        mock_client->set_response_body("{}");
        mock_client->set_status_code(400);
        
        Employee created_employee;
        auto result = api.createEmployee(input, created_employee);
        
        REQUIRE_FALSE(result.success);
    }
}

TEST_CASE("EmployeeApi getEmployeeById", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Successful retrieval") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": {
                "id": "123",
                "employee_name": "Mahesh Dekhle",
                "employee_salary": 75000,
                "employee_age": 30,
                "employee_title": "Software Engineer",
                "employee_email": "md@example.com"
            }
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        Employee employee;
        auto result = api.getEmployeeById("123", employee);
        
        REQUIRE(result.success);
        REQUIRE(employee.getId() == "123");
        REQUIRE(employee.getName() == "Mahesh Dekhle");
    }
    
    SECTION("Employee not found should return empty optional") 
    {
        mock_client->set_response_body("{}");
        mock_client->set_status_code(404);
        
        Employee employee;
        auto result = api.getEmployeeById("999", employee);
        
        REQUIRE_FALSE(result.success);
    }
    
    SECTION("Network error should return empty optional") 
    {
        mock_client->set_should_fail(true);
        
        Employee employee;
        auto result = api.getEmployeeById("123", employee);
        
        REQUIRE_FALSE(result.success);
    }
}

TEST_CASE("EmployeeApi deleteEmployeeById", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Successful deletion") 
    {
        // First, set up getAllEmployees to return the employee to be deleted
        std::string employees_response = R"({
            "status": "success",
            "data": [
                {
                    "id": "123",
                    "employee_name": "Mahesh Dekhle",
                    "employee_salary": 75000,
                    "employee_age": 30,
                    "employee_title": "Software Engineer",
                    "employee_email": "md@example.com"
                }
            ]
        })";
        
        mock_client->set_response_body(employees_response);
        mock_client->set_status_code(200);
        
        // Set up the delete response
        std::string delete_response = R"({
            "status": "success",
            "data": true
        })";
        
        mock_client->set_delete_response_body(delete_response);
        
        std::string employee_name;
        auto result = api.deleteEmployeeById("123", employee_name);
        
        REQUIRE(result.success);
        REQUIRE(employee_name == "Mahesh Dekhle");
    }
    
    SECTION("Employee not found should return empty optional") 
    {
        // Set up getAllEmployees to return empty list
        std::string employees_response = R"({
            "status": "success",
            "data": []
        })";
        
        mock_client->set_response_body(employees_response);
        mock_client->set_status_code(200);
        
        std::string employee_name;
        auto result = api.deleteEmployeeById("999", employee_name);
        
        REQUIRE_FALSE(result.success);
    }
    
    SECTION("Network error should return empty optional") 
    {
        mock_client->set_should_fail(true);
        
        std::string employee_name;
        auto result = api.deleteEmployeeById("123", employee_name);
        
        REQUIRE_FALSE(result.success);
    }
}

TEST_CASE("EmployeeApi getAllEmployees", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Successful retrieval of multiple employees") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": [
                {
                    "id": "123",
                    "employee_name": "Mahesh Dekhle",
                    "employee_salary": 75000,
                    "employee_age": 30,
                    "employee_title": "Software Engineer",
                    "employee_email": "md@example.com"
                },
                {
                    "id": "456",
                    "employee_name": "Kaustab Kharche",
                    "employee_salary": 80000,
                    "employee_age": 28,
                    "employee_title": "Senior Developer",
                    "employee_email": "kk@example.com"
                }
            ]
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        std::vector<Employee> employees;
        auto result = api.getAllEmployees(employees);
        REQUIRE(result.success);
        REQUIRE(employees.size() == 2);
        REQUIRE(employees[0].getId() == "123");
        REQUIRE(employees[0].getName() == "Mahesh Dekhle");
        REQUIRE(employees[1].getId() == "456");
        REQUIRE(employees[1].getName() == "Kaustab Kharche");
    }
    
    SECTION("Empty array response") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": []
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        std::vector<Employee> employees;
        auto result = api.getAllEmployees(employees);
        REQUIRE(result.success);
        REQUIRE(employees.empty());
    }
    
    SECTION("Network error should return empty vector") 
    {
        mock_client->set_should_fail(true);
        
        std::vector<Employee> employees;
        auto result = api.getAllEmployees(employees);
        REQUIRE_FALSE(result.success);
        REQUIRE(result.error_type == ApiErrorType::NETWORK_ERROR);
    }
}

TEST_CASE("EmployeeApi getEmployeeByNameSearch", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Successful search") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": [
                {
                    "id": "123",
                    "employee_name": "Mahesh Dekhle",
                    "employee_salary": 75000,
                    "employee_age": 30,
                    "employee_title": "Software Engineer",
                    "employee_email": "md@example.com"
                }
            ]
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        std::vector<Employee> employees;
        auto result = api.getEmployeeByNameSearch("Mahesh", employees);
        REQUIRE(result.success);
        REQUIRE(employees.size() == 1);
        REQUIRE(employees[0].getName() == "Mahesh Dekhle");
    }
    
    SECTION("No matches found") {
        std::string response_json = R"({
            "status": "success",
            "data": []
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        std::vector<Employee> employees;
        auto result = api.getEmployeeByNameSearch("Nonexistent", employees);
        REQUIRE(result.success);
        REQUIRE(employees.empty());
    }
}

TEST_CASE("EmployeeApi getTop10HighestEarningEmployeeNames", "[employee_api]") {
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Successful retrieval") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": [
                {
                    "id": "123",
                    "employee_name": "Mahesh Dekhle",
                    "employee_salary": 75000,
                    "employee_age": 30,
                    "employee_title": "Software Engineer",
                    "employee_email": "md@example.com"
                },
                {
                    "id": "456",
                    "employee_name": "Kaustab Kharche",
                    "employee_salary": 80000,
                    "employee_age": 28,
                    "employee_title": "Senior Developer",
                    "employee_email": "kk@example.com"
                },
                {
                    "id": "789",
                    "employee_name": "Rajesh Kumar",
                    "employee_salary": 65000,
                    "employee_age": 35,
                    "employee_title": "QA Engineer",
                    "employee_email": "rk@example.com"
                }
            ]
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        std::vector<std::string> names;
        auto result = api.getTop10HighestEarningEmployeeNames(names);
        REQUIRE(result.success);
        REQUIRE(names.size() == 3);
        REQUIRE(names[0] == "Kaustab Kharche"); // Highest salary
        REQUIRE(names[1] == "Mahesh Dekhle");
        REQUIRE(names[2] == "Rajesh Kumar");
    }
    
    SECTION("Empty array response") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": []
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        std::vector<std::string> names;
        auto result = api.getTop10HighestEarningEmployeeNames(names);
        REQUIRE(result.success);
        REQUIRE(names.empty());
    }
}

TEST_CASE("EmployeeApi getHighestSalaryFromEmployees", "[employee_api]") {
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Successful retrieval") 
    {
        std::string response_json = R"({
            "status": "success",
            "data": [
                {
                    "id": "123",
                    "employee_name": "Mahesh Dekhle",
                    "employee_salary": 75000,
                    "employee_age": 30,
                    "employee_title": "Software Engineer",
                    "employee_email": "md@example.com"
                },
                {
                    "id": "456",
                    "employee_name": "Kaustab Kharche",
                    "employee_salary": 80000,
                    "employee_age": 28,
                    "employee_title": "Senior Developer",
                    "employee_email": "kk@example.com"
                }
            ]
        })";
        
        mock_client->set_response_body(response_json);
        mock_client->set_status_code(200);
        
        int highest_salary;
        auto result = api.getHighestSalaryFromEmployees(highest_salary);
        REQUIRE(result.success);
        REQUIRE(highest_salary == 80000);
    }
}

TEST_CASE("EmployeeApi URL construction", "[employee_api]") 
{
    auto mock_client = std::make_shared<MockEmployeeHttpClient>();
    EmployeeApi api(mock_client, "https://api.example.com");
    
    SECTION("Base URL with trailing slash") {
        EmployeeApi api_with_slash(mock_client, "https://api.example.com/");
        // Should not throw
        REQUIRE_NOTHROW(api_with_slash);
    }
    
    SECTION("Base URL without trailing slash") {
        EmployeeApi api_without_slash(mock_client, "https://api.example.com");
        // Should not throw
        REQUIRE_NOTHROW(api_without_slash);
    }
} 