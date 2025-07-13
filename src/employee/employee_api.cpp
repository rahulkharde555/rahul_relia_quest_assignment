#include "employee/employee_api.h"
#include "http_client/http_client.h"
#include "http_client/retry_http_client.h"
#include "employee/employee_json.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <optional>
#include "http_client/url_encode.h"
#include "utils/logger.h"
#include "utils/http_response_utils.h"
#include "utils/validation_utils.h"


EmployeeApi::EmployeeApi(
    std::shared_ptr<IHttpClient> _http_client,
    std::string _base_url,
    int _timeout_ms
) : 
http_client(std::move(_http_client)), 
base_url(std::move(_base_url)), 
timeout_ms(_timeout_ms) {}


ApiResponse EmployeeApi::getAllEmployees(std::vector<Employee>& employees) const 
{
    try 
    {
        HTTPRequest req(HttpMethod::GET, base_url + "employee", {{"Accept", "application/json"}}, "", 2000, timeout_ms);
        
        if (!req.is_valid()) 
        {
            LOG_ERROR("Invalid HTTP request: " + req.get_validation_error());
            return ApiResponse(ApiErrorType::VALIDATION_ERROR, req.get_validation_error());
        }
        
        RetryHttpClient retryClient(http_client, 3, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error)
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            LOG_WARN("Empty response received from API");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
        }

        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "getAllEmployees");
        if (validation_error) 
        {
            return *validation_error;
        }

        nlohmann::json data = json_response["data"];
        if (!data.is_array())
        {
            LOG_ERROR("API returned non-array data");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "API returned non-array data");
        }

        employees.clear();
        for (const auto& employee : data) 
        {
            try 
            {
                Employee emp;
                from_json(employee, emp);
                employees.push_back(emp);
            } catch (const std::exception& e) 
            {
                LOG_ERROR(std::string("Failed to deserialize employee data: ") + e.what());
                continue;
            }
        }
        return ApiResponse::success_response("Successfully retrieved " + std::to_string(employees.size()) + " employees");

    } catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in getAllEmployees: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in getAllEmployees: " + std::string(e.what()));
    }
}

ApiResponse EmployeeApi::getEmployeeById(const std::string& id, Employee& employee) const 
{
    auto validation_error = validateEmployeeId(id);
    if (validation_error) 
    {
        return *validation_error;
    }

    try 
    {
        HTTPRequest req(HttpMethod::GET, base_url + "employee/" + id, {{"Accept", "application/json"}}, "", 2000, timeout_ms);
        
        auto req_error = validateRequest(req, "getEmployeeById");
        if (req_error) 
        {
            return *req_error;
        }
        
        RetryHttpClient retryClient(http_client, 3, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error)
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            LOG_WARN("Empty response received from API");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
        }

        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "getEmployeeById");
        if (validation_error) 
        {
            return *validation_error;
        }

        nlohmann::json data = json_response["data"];
        if (data.is_null()) 
        {
            LOG_WARN("API returned null data");
            return ApiResponse(ApiErrorType::NOT_FOUND, "Employee not found");
        }

        try 
        {
            from_json(data, employee);
        } catch (const std::exception& e)
        {
            LOG_ERROR(std::string("Failed to deserialize employee data: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to deserialize employee data: " + std::string(e.what()));
        }

        return ApiResponse::success_response("Successfully retrieved employee");

    } catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in getEmployeeById: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in getEmployeeById: " + std::string(e.what()));
    }
}

ApiResponse EmployeeApi::getEmployeeByNameSearch(const std::string& name, std::vector<Employee>& employees) const 
{
    auto validation_error = validateSearchName(name);
    if (validation_error) {
        return *validation_error;
    }

    try 
    {
        HTTPRequest req(HttpMethod::GET, base_url + "employee", {{"Accept", "application/json"}}, "", 2000, timeout_ms);
        
        auto req_error = validateRequest(req, "getEmployeeByNameSearch");
        if (req_error) 
        {
            return *req_error;
        }
        
        RetryHttpClient retryClient(http_client, 3, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error) 
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            LOG_WARN("Empty response received from API");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
        }

        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "getEmployeeByNameSearch");
        if (validation_error) 
        {
            return *validation_error;
        }

        nlohmann::json data = json_response["data"];
        if (!data.is_array()) 
        {
            LOG_ERROR("API returned non-array data");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "API returned non-array data");
        }

        employees.clear();
        for (const auto& employee : data) 
        {
            try 
            {
                Employee emp;
                from_json(employee, emp);

                if (emp.getName().find(name) != std::string::npos) 
                {
                    employees.push_back(emp);
                }
            } catch (const std::exception& e) 
            {
                LOG_ERROR(std::string("Failed to deserialize employee data: ") + e.what());
                continue;
            }
        }
        return ApiResponse::success_response("Found " + std::to_string(employees.size()) + " employees matching '" + name + "'");

    } catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in getEmployeeByNameSearch: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in getEmployeeByNameSearch: " + std::string(e.what()));
    }
}

ApiResponse EmployeeApi::getTop10HighestEarningEmployeeNames(std::vector<std::string>& names) const 
{
    try 
    {
        HTTPRequest req(HttpMethod::GET, base_url + "employee", {{"Accept", "application/json"}}, "", 2000, timeout_ms);
        
        auto req_error = validateRequest(req, "getTop10HighestEarningEmployeeNames");
        if (req_error) 
        {
            return *req_error;
        }
        
        RetryHttpClient retryClient(http_client, 3, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error) 
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            LOG_WARN("Empty response received from API");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
        }

        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } 
        catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "getTop10HighestEarningEmployeeNames");
        if (validation_error) 
        {
            return *validation_error;
        }

        nlohmann::json data = json_response["data"];
        if (!data.is_array()) 
        {
            LOG_ERROR("API returned non-array data");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "API returned non-array data");
        }

        std::sort(data.begin(), data.end(), [](const nlohmann::json& a, const nlohmann::json& b) 
        {
            try 
            {
                return a["employee_salary"].get<int>() > b["employee_salary"].get<int>();
            } 
            catch (const std::exception& e) 
            {
                return false;
            }
        });

        names.clear();
        for (int i = 0; i < 10 && i < data.size(); i++) 
        {
            try 
            {
                std::string name = data[i]["employee_name"].get<std::string>();
                names.push_back(name);
            } 
            catch (const std::exception& e) 
            {
                continue;
            }
        }
        return ApiResponse::success_response("Retrieved top " + std::to_string(names.size()) + " highest earning employees");

    } catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in getTop10HighestEarningEmployeeNames: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in getTop10HighestEarningEmployeeNames: " + std::string(e.what()));
    }
}

ApiResponse EmployeeApi::getHighestSalaryFromEmployees(int& highest_salary) const 
{
    try 
    {
        HTTPRequest req(HttpMethod::GET, base_url + "employee", {{"Accept", "application/json"}}, "", 2000, timeout_ms);
        
        auto req_error = validateRequest(req, "getHighestSalaryFromEmployees");
        if (req_error) 
        {
            return *req_error;
        }
        
        RetryHttpClient retryClient(http_client, 3, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error) 
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            LOG_WARN("Empty response received from API");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
        }

        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } 
        catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "getHighestSalaryFromEmployees");
        if (validation_error) 
        {
            return *validation_error;
        }

        nlohmann::json data = json_response["data"];
        if (!data.is_array()) 
        {
            LOG_ERROR("API returned non-array data");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "API returned non-array data");
        }

        if (data.empty()) 
        {
            return ApiResponse(ApiErrorType::NOT_FOUND, "No employees found");
        }

        std::sort(data.begin(), data.end(), [](const nlohmann::json& a, const nlohmann::json& b) 
        {
            try 
            {
                return a["employee_salary"].get<int>() > b["employee_salary"].get<int>();
            } catch (const std::exception& e) 
            {
                return false;
            }
        }); 

        try 
        {
            highest_salary = data[0]["employee_salary"].get<int>();
            return ApiResponse::success_response("Highest salary: " + std::to_string(highest_salary));
        } 
        catch (const std::exception& e) 
        {
            LOG_ERROR(std::string("Failed to get highest salary: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to get highest salary: " + std::string(e.what()));
        }

    } 
    catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in getHighestSalaryFromEmployees: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in getHighestSalaryFromEmployees: " + std::string(e.what()));
    }
}

ApiResponse EmployeeApi::createEmployee(const Employee& employee, Employee& created_employee) const 
{
    auto validation_error = validateEmployeeData(employee);
    if (validation_error) 
    {
        return *validation_error;
    }

    try 
    {
        nlohmann::json json_employee;
        to_json_create_employee(json_employee, employee);

        HTTPRequest req(HttpMethod::POST, base_url + "employee", 
                       {{"Accept", "application/json"}, {"Content-Type", "application/json"}}, 
                       json_employee.dump(), 2000, timeout_ms);
        
        auto req_error = validateRequest(req, "createEmployee");
        if (req_error)
        {
            return *req_error;
        }
        
        RetryHttpClient retryClient(http_client, 3, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error) 
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            LOG_WARN("Empty response received from API");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
        }

        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } 
        catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "createEmployee");
        if (validation_error) 
        {
            return *validation_error;
        }

        nlohmann::json data = json_response["data"];
        if (data.is_null()) 
        {
            LOG_WARN("API returned null data");
            return ApiResponse(ApiErrorType::PARSE_ERROR, "API returned null data");
        }

        try 
        {
            from_json(data, created_employee);
        } 
        catch (const std::exception& e) 
        {
            LOG_ERROR(std::string("Failed to deserialize employee data: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to deserialize employee data: " + std::string(e.what()));
        }

        return ApiResponse::success_response("Successfully created employee");

    } 
    catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in createEmployee: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in createEmployee: " + std::string(e.what()));
    }
}


ApiResponse EmployeeApi::deleteEmployeeById(const std::string& id, std::string& employee_name) const 
{
    auto validation_error = validateEmployeeId(id);
    if (validation_error) 
    {
        return *validation_error;
    }

    try 
    {
        std::vector<Employee> employees;
        auto employees_result = getAllEmployees(employees);
        if (!employees_result.success) 
        {
            return employees_result;
        }

        for (const auto& employee : employees) 
        {
            if (employee.getId() == id) 
            {
                employee_name = employee.getName();
                break;
            }
        }

        if(employee_name.empty()) 
        {
            return ApiResponse(ApiErrorType::NOT_FOUND, "Employee not found");
        }

        nlohmann::json json_delete_employee;
        json_delete_employee["name"] = employee_name;
        std::cerr << "json_delete_employee: " << json_delete_employee.dump() << std::endl;

        HTTPRequest req(HttpMethod::DELETE, base_url + "employee", 
            {{"Accept", "application/json"}, {"Content-Type", "application/json"}}, 
            json_delete_employee.dump(), 2000, timeout_ms);
        
        auto req_error = validateRequest(req, "deleteEmployeeById");
        if (req_error) 
        {
            return *req_error;
        }
        
        RetryHttpClient retryClient(http_client, 1, 1000);
        HTTPResponse resp = retryClient.request(req);
        
        auto error = handleHttpResponse(req, resp);
        if (error) 
        {
            return *error;
        }

        std::string response = resp.get_body();
        if (response.empty()) 
        {
            if (resp.get_status_code() == 204) 
            {
                return ApiResponse::success_response("Employee deleted successfully");
            } 
            else 
            {
                return ApiResponse(ApiErrorType::PARSE_ERROR, "Empty response received from API");
            }
        }

        
        nlohmann::json json_response;
        try 
        {
            json_response = nlohmann::json::parse(response);
        } 
        catch (const nlohmann::json::parse_error& e) 
        {
            LOG_ERROR(std::string("Failed to parse JSON response: ") + e.what());
            return ApiResponse(ApiErrorType::PARSE_ERROR, "Failed to parse JSON response: " + std::string(e.what()));
        }

        auto validation_error = validateJsonResponse(json_response, "deleteEmployeeById");
        if (validation_error) 
        {
            return *validation_error;
        }

        bool data = json_response["data"];
        if (data) 
        {
            return ApiResponse::success_response("Employee deleted successfully");
        } 
        else 
        {
            return ApiResponse(ApiErrorType::SERVER_ERROR, "Delete operation failed");
        }

    } 
    catch (const std::exception& e) 
    {
        LOG_ERROR(std::string("Unexpected error in deleteEmployeeById: ") + e.what());
        return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unexpected error in deleteEmployeeById: " + std::string(e.what()));
    }
}
