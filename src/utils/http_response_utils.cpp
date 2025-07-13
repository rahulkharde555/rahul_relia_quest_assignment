#include "utils/http_response_utils.h"
#include "utils/logger.h"

std::optional<ApiResponse> handleHttpResponse(const HTTPRequest& req, const HTTPResponse& resp) 
{
    if (resp.is_success()) 
    {
        return std::nullopt;
    }
    
    LOG_ERROR("Request failed: " + std::to_string(static_cast<int>(req.get_method())) + " " + req.get_url());
    
    for (const auto& header : req.get_headers()) 
    {
        LOG_ERROR("  Header: " + header.first + ": " + header.second);
    }

    if (!req.get_body().empty()) 
    {
        LOG_ERROR("  Body: " + req.get_body());
    }
    
    LOG_ERROR("Response status: " + std::to_string(resp.get_status_code()));
    
    for (const auto& header : resp.get_headers()) 
    {
        LOG_ERROR("  Header: " + header.first + ": " + header.second);
    }

    if (!resp.get_body().empty()) 
    {
        LOG_ERROR("  Body: " + resp.get_body());
    }
    
    if (resp.is_network_error()) 
    {
        LOG_ERROR("Network error: " + resp.get_error() + " (Type: " + std::to_string(static_cast<int>(resp.get_error_type())) + ")");
        return ApiResponse(ApiErrorType::NETWORK_ERROR, resp.get_error(), resp.get_status_code());
    } 
    else if (resp.is_client_error()) 
    {
        LOG_WARN("Client error (4xx): " + std::to_string(resp.get_status_code()));

        if (resp.get_status_code() == 404) 
        {
            return ApiResponse(ApiErrorType::NOT_FOUND, "Resource not found", resp.get_status_code());
        } 
        else if (resp.get_status_code() == 429) 
        {
            LOG_WARN("Rate limit exceeded (429): Too many requests");
            return ApiResponse(ApiErrorType::RATE_LIMIT_ERROR, "Rate limit exceeded - too many requests", resp.get_status_code());
        } 
        else if (resp.get_status_code() == 400) 
        {
            return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Bad request - invalid input data", resp.get_status_code());
        } 
        else if (resp.get_status_code() == 408) 
        {
            return ApiResponse(ApiErrorType::TIMEOUT_ERROR, "Request timeout", resp.get_status_code());
        } 
        else 
        {
            return ApiResponse(ApiErrorType::VALIDATION_ERROR, "Client error: " + std::to_string(resp.get_status_code()), resp.get_status_code());
        }
    } 
    else if (resp.is_server_error()) 
    {
        LOG_ERROR("Server error (5xx): " + std::to_string(resp.get_status_code()));
        return ApiResponse(ApiErrorType::SERVER_ERROR, "Server error: " + std::to_string(resp.get_status_code()), resp.get_status_code());

    }
    return ApiResponse(ApiErrorType::UNKNOWN_ERROR, "Unknown error with status code: " + std::to_string(resp.get_status_code()), resp.get_status_code());
} 