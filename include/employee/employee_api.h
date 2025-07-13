#pragma once
#include "employee/employee.h"
#include "http_client/http_client.h"
#include <vector>
#include <optional>
#include <string>

enum class ApiErrorType 
{
    NETWORK_ERROR,
    NOT_FOUND,
    VALIDATION_ERROR,
    RATE_LIMIT_ERROR,
    SERVER_ERROR,
    PARSE_ERROR,
    TIMEOUT_ERROR,
    UNKNOWN_ERROR
};

struct ApiResponse 
{
    bool success;
    std::string message;
    ApiErrorType error_type;
    int status_code;
    
    // Success constructor
    ApiResponse() : 
        success(true), 
        message("Success"), 
        error_type(ApiErrorType::UNKNOWN_ERROR), 
        status_code(0) {}
    
    // Error constructor
    ApiResponse(ApiErrorType type, std::string msg, int code = 0) : 
        success(false), 
        message(std::move(msg)), 
        error_type(type), 
        status_code(code) {}
    
    // Success with custom message
    ApiResponse(std::string msg) : 
        success(true), 
        message(std::move(msg)), 
        error_type(ApiErrorType::UNKNOWN_ERROR), 
        status_code(0) {}
    
    static ApiResponse success_response()
    { 
        return ApiResponse(); 
    }
    static ApiResponse success_response(std::string msg) 
    { 
        return ApiResponse(std::move(msg)); 
    }
    static ApiResponse error_response(ApiErrorType type, std::string msg, int code = 0) 
    { 
        return ApiResponse(type, std::move(msg), code); 
    }
    explicit operator bool() const 
    { 
        return success; 
    }
};

class EmployeeApi 
{
    public:
        EmployeeApi() = default;
        ~EmployeeApi() = default;
        EmployeeApi(
            std::shared_ptr<IHttpClient> _http_client,
            std::string _base_url = "http://localhost:8112/api/v1/employee",
            int _timeout_ms = 5000
        );

   //APIs: 
   ApiResponse getAllEmployees(std::vector<Employee>& employees) const;

   ApiResponse getEmployeeById(const std::string& id, Employee& employee) const;

   ApiResponse createEmployee(const Employee& employee, Employee& created_employee) const;

   ApiResponse deleteEmployeeById(const std::string& id, std::string& employee_name) const;

   ApiResponse getEmployeeByNameSearch(const std::string& name, std::vector<Employee>& employees) const;

   ApiResponse getTop10HighestEarningEmployeeNames(std::vector<std::string>& names) const;

   ApiResponse getHighestSalaryFromEmployees(int& highest_salary) const;

   // Configuration methods
   void setTimeout(int timeout_ms);
   int getTimeout() const;
   void setRetryConfig(int max_retries, int base_backoff_ms);
   std::pair<int, int> getRetryConfig() const;
   
   // Error recovery configuration
   void setRetryOnRateLimit(bool retry_on_rate_limit);
   bool getRetryOnRateLimit() const;
   void setRetryOnServerError(bool retry_on_server_error);
   bool getRetryOnServerError() const;

private:
    std::shared_ptr<IHttpClient> http_client;
    std::string base_url;
    int timeout_ms;
    
};

