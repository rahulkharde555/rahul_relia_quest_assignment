#include "utils/validation_utils.h"
#include "utils/logger.h"

std::optional<ApiResponse> validateRequest(const HTTPRequest& req, const std::string& operation) 
{
    if (!req.is_valid()) 
    {
        LOG_ERROR("Invalid HTTP request for " + operation + ": " + req.get_validation_error());
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, req.get_validation_error());
    }
    return std::nullopt;
}

std::optional<ApiResponse> validateEmployeeId(const std::string& id) 
{
    if (id.empty()) 
    {
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Employee ID cannot be empty");
    }
    return std::nullopt;
}

std::optional<ApiResponse> validateEmployeeData(const Employee& employee) 
{
    if (employee.getName().empty()) 
    {
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Employee name cannot be empty");
    }
    if (employee.getSalary() <= 0) 
    {
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Employee salary cannot be less than or equal to 0");
    }
    if (employee.getAge() < 16 || employee.getAge() > 75) 
    {
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Employee age must be between 16 and 75");
    }
    if (employee.getTitle().empty()) 
    {
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Employee title cannot be empty");
    }
    return std::nullopt;
}

std::optional<ApiResponse> validateSearchName(const std::string& name) 
{
    if (name.empty()) 
    {
        return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Search name cannot be empty");
    }
    return std::nullopt;
}

std::optional<ApiResponse> validateJsonResponse(const nlohmann::json& json_response, const std::string& operation) 
{
    if (!json_response.contains("status") || !json_response.contains("data")) 
    {
        LOG_ERROR("Invalid JSON response structure for " + operation + ": missing 'status' or 'data' field");
        return ApiResponse(ApiErrorType::PARSE_ERROR, "Invalid JSON response structure: missing 'status' or 'data' field");
    }
    return std::nullopt;
} 