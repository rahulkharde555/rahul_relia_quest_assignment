#include "http_client/http_client.h"
#include <cpr/cpr.h>
#include <stdexcept>
#include <iostream>
#include <regex>
#include <algorithm>
#include "utils/logger.h"

bool HTTPRequest::is_valid() const 
{
    return get_validation_error().empty();
}

std::string HTTPRequest::get_validation_error() const 
{
    if (url.empty()) 
    {
        return "URL cannot be empty";
    }
    
    if (connect_timeout_ms <= 0) 
    {
        return "Connect timeout must be greater than 0";
    }
    
    if (total_timeout_ms <= 0) 
    {
        return "Total timeout must be greater than 0";
    }
    
    if (connect_timeout_ms > total_timeout_ms) 
    {
        return "Connect timeout cannot be greater than total timeout";
    }
    
    if (!validate_url_format(url)) 
    {
        return "Invalid URL format";
    }
    
    return ""; // No validation errors
}

bool validate_url_format(const std::string& url) 
{
    if (url.empty()) 
    {
        return false;
    }
    
    std::regex url_pattern(R"(^(https?://)[a-zA-Z0-9\-._~:/?#[\]@!$&'()*+,;=%]+$)");
    return std::regex_match(url, url_pattern);
}

HttpErrorType HTTPClient::categorize_cpr_error(const std::string& cpr_error) const 
{
    if (cpr_error.empty()) 
    {
        return HttpErrorType::NONE;
    }
    
    std::string error_lower = cpr_error;
    std::transform(error_lower.begin(), error_lower.end(), error_lower.begin(), ::tolower);
    
    if (error_lower.find("timeout") != std::string::npos) 
    {
        if (error_lower.find("connect") != std::string::npos) 
        {
            return HttpErrorType::CONNECTION_TIMEOUT;
        }
        return HttpErrorType::REQUEST_TIMEOUT;
    }
    
    if (error_lower.find("connection refused") != std::string::npos ||
        error_lower.find("connection failed") != std::string::npos) 
    {
        return HttpErrorType::CONNECTION_REFUSED;
    }
    
    if (error_lower.find("network") != std::string::npos) 
    {
        return HttpErrorType::NETWORK_ERROR;
    }
    
    return HttpErrorType::UNKNOWN_ERROR;
}

std::string HTTPClient::sanitize_url(const std::string& url) const 
{
    std::string sanitized = url;
    
    sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\0'), sanitized.end());
    
    sanitized.erase(0, sanitized.find_first_not_of(" \t\r\n"));
    sanitized.erase(sanitized.find_last_not_of(" \t\r\n") + 1);
    
    return sanitized;
}

HTTPResponse HTTPClient::request(const HTTPRequest& request) const 
{
    LOG_DEBUG("Starting HTTP request to: " + request.get_url());
    
    try 
    {
        if (!request.is_valid()) 
        {
            std::string validation_error = request.get_validation_error();
            // std::cerr << "Request validation failed: " << validation_error << std::endl;
            LOG_ERROR("Request validation failed: " + validation_error);
            return HTTPResponse(0, "", {}, validation_error, HttpErrorType::INVALID_REQUEST);
        }

        std::string sanitized_url = sanitize_url(request.get_url());
        if (sanitized_url != request.get_url()) 
        {
            // std::cerr << "URL sanitized from: " << request.get_url() << " to: " << sanitized_url << std::endl;
            LOG_DEBUG("URL sanitized from: " + request.get_url() + " to: " + sanitized_url);
        }
        
        if (!validate_url_format(sanitized_url)) 
        {
            // std::cerr << "Invalid URL format: " << sanitized_url << std::endl;
            LOG_ERROR("Invalid URL format: " + sanitized_url);
            return HTTPResponse(0, "", {}, "Invalid URL format", HttpErrorType::INVALID_URL);
        }

        //Log the request 
        // std::cout << "Sending request to " << sanitized_url << std::endl;
        // std::cout << "Method: " << static_cast<int>(request.get_method()) << std::endl;
        // std::cout << "Headers: " << request.get_headers().size() << " headers" << std::endl;
        // std::cout << "Body length: " << request.get_body().length() << std::endl;
        // std::cout << "Connect timeout: " << request.get_connect_timeout_ms() << "ms" << std::endl;
        // std::cout << "Total timeout: " << request.get_total_timeout_ms() << "ms" << std::endl;
        
        LOG_INFO(std::string("Sending ") + 
                     (request.get_method() == HttpMethod::GET ? "GET" :
                     request.get_method() == HttpMethod::POST ? "POST" :
                     request.get_method() == HttpMethod::PUT ? "PUT" : "DELETE") +
                     " request to: " + sanitized_url);
        LOG_DEBUG("Request details - Headers: " + std::to_string(request.get_headers().size()) + 
                      ", Body length: " + std::to_string(request.get_body().length()) + 
                      ", Connect timeout: " + std::to_string(request.get_connect_timeout_ms()) + "ms" +
                      ", Total timeout: " + std::to_string(request.get_total_timeout_ms()) + "ms");

        cpr::Header cpr_headers;
        for (const auto& [header_key, header_value] : request.get_headers()) {
            if (!header_key.empty() && !header_value.empty()) {
                cpr_headers.insert({header_key, header_value});
            } else {
                // std::cerr << "Warning: Skipping empty header key or value" << std::endl;
                LOG_WARN("Warning: Skipping empty header key or value");
            }
        }

        cpr::ConnectTimeout connect_timeout{request.get_connect_timeout_ms()};
        cpr::Timeout total_timeout{request.get_total_timeout_ms()};

        cpr::Response response;
        try 
        {
            switch (request.get_method()) 
            {
                case HttpMethod::GET:
                    response = cpr::Get(cpr::Url{sanitized_url}, cpr_headers, connect_timeout, total_timeout);
                    break;
                case HttpMethod::POST:
                    response = cpr::Post(cpr::Url{sanitized_url}, cpr_headers, cpr::Body{request.get_body()}, connect_timeout, total_timeout);
                    break;
                case HttpMethod::PUT:
                    response = cpr::Put(cpr::Url{sanitized_url}, cpr_headers, cpr::Body{request.get_body()}, connect_timeout, total_timeout);
                    break;
                case HttpMethod::DELETE:
                    response = cpr::Delete(cpr::Url{sanitized_url}, cpr_headers, cpr::Body{request.get_body()}, connect_timeout, total_timeout);
                    break;
                default:
                    // std::cerr << "Invalid HTTP method: " << static_cast<int>(request.get_method()) << std::endl;
                    LOG_ERROR("Invalid HTTP method: " + std::to_string(static_cast<int>(request.get_method())));
                    return HTTPResponse(0, "", {}, "Invalid HTTP method", HttpErrorType::INVALID_METHOD);
            }
        } 
        catch (const std::exception& e) 
        {
            // std::cerr << "CPR request failed with exception: " << e.what() << std::endl;
            LOG_ERROR(std::string("CPR request failed with exception: ") + e.what());
            return HTTPResponse(0, "", {}, std::string("CPR exception: ") + e.what(), HttpErrorType::NETWORK_ERROR);
        }

        // std::cout << "Response status code: " << response.status_code << std::endl;
        // std::cout << "Response body length: " << response.text.length() << std::endl;
        // if (!response.error.message.empty()) {
        //     std::cout << "CPR error: " << response.error.message << std::endl;
        // }
        
        LOG_INFO("Received response - Status: " + std::to_string(response.status_code) + 
                     ", Body length: " + std::to_string(response.text.length()) + 
                     ", Headers: " + std::to_string(response.header.size()));
        if (!response.error.message.empty()) 
        {
            LOG_ERROR("CPR error: " + response.error.message);
        }

        std::map<std::string, std::string> resp_headers;
        for (const auto& [k, v] : response.header) {
            if (!k.empty()) {
                resp_headers[k] = v;
            }
        }

        HttpErrorType error_type = HttpErrorType::NONE;
        std::string error_message = "";
        
        if (response.error) 
        {
            error_type = categorize_cpr_error(response.error.message);
            error_message = response.error.message;
            // std::cerr << "Request failed with error type: " << static_cast<int>(error_type) << ", message: " << error_message << std::endl;
            LOG_ERROR("Request failed with error type: " + std::to_string(static_cast<int>(error_type)) + ", message: " + error_message);
        } 
        else if (response.status_code == 0) 
        {
            error_type = HttpErrorType::NETWORK_ERROR;
            error_message = "No response received from server";
            // std::cerr << "No response received from server" << std::endl;
            LOG_ERROR("No response received from server");
        }

        if (response.status_code < 0) 
        {
            error_type = HttpErrorType::NETWORK_ERROR;
            error_message = "Invalid status code received";
            // std::cerr << "Invalid status code received: " << response.status_code << std::endl;
            LOG_ERROR("Invalid status code received: " + std::to_string(response.status_code));
        }

        HTTPResponse result = HTTPResponse(
            response.status_code,
            response.text,
            resp_headers,
            error_message,
            error_type
        );
        
        if (result.is_success()) 
        {
            // std::cout << "Request completed successfully" << std::endl;
            LOG_INFO("Request completed successfully");
        } 
        else 
        {
            // std::cout << "Request completed with errors - Status: " << result.get_status_code() << ", Error: " << result.get_error() << std::endl;
            LOG_WARN("Request completed with errors - Status: " + std::to_string(result.get_status_code()) + 
                         ", Error: " + result.get_error());
        }
        
        return result;

    } 
    catch (const std::exception& e) 
    {
        // std::cerr << "Unexpected error in HTTPClient::request: " << e.what() << std::endl;
        LOG_ERROR(std::string("Unexpected error in HTTPClient::request: ") + e.what());
        return HTTPResponse(0, "", {}, std::string("Unexpected error: ") + e.what(), HttpErrorType::UNKNOWN_ERROR);
    }
} 