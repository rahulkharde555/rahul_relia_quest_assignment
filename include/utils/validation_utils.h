#pragma once
#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include "employee/employee.h"
#include "http_client/http_client.h"
#include "employee/employee_api.h" // for ApiResponse, ApiErrorType

std::optional<ApiResponse> validateRequest(const HTTPRequest& req, const std::string& operation);
std::optional<ApiResponse> validateEmployeeId(const std::string& id);
std::optional<ApiResponse> validateEmployeeData(const Employee& employee);
std::optional<ApiResponse> validateSearchName(const std::string& name);
std::optional<ApiResponse> validateJsonResponse(const nlohmann::json& json_response, const std::string& operation); 