#pragma once
#include <optional>
#include "http_client/http_client.h"
#include "employee/employee.h"
#include <string>
#include <nlohmann/json.hpp>
#include "employee/employee_api.h" // for ApiResponse, ApiErrorType

std::optional<ApiResponse> handleHttpResponse(const HTTPRequest& req, const HTTPResponse& resp); 