#pragma once
#include "http_client.h"
#include <memory>
#include <chrono>
#include <thread>
#include <random>
#include "utils/logger.h"

class RetryHttpClient : public IHttpClient 
{
public:
    RetryHttpClient(
        std::shared_ptr<IHttpClient> inner, 
        int max_retries = 3, 
        int base_backoff_ms = 1000)
        : 
        inner_http_client(std::move(inner)), 
        max_retries(max_retries), 
        base_backoff_ms(base_backoff_ms) {}

    HTTPResponse request(const HTTPRequest& request) const override 
    {
        int attempt = 0;
        HTTPResponse last_response;
        
        LOG_INFO("Starting request with retry logic - Max retries: " + std::to_string(max_retries) + ", Base backoff: " + std::to_string(base_backoff_ms) + "ms");
        
        while (attempt <= max_retries) 
        {
            LOG_DEBUG("Making request attempt " + std::to_string(attempt + 1) + "/" + std::to_string(max_retries + 1));
            HTTPResponse response = inner_http_client->request(request);
            last_response = response;
            
            if (response.is_success()) {
                LOG_INFO("Request succeeded on attempt " + std::to_string(attempt + 1) + "/" + std::to_string(max_retries + 1));
                return response;
            }
            
            if (!should_retry(response, attempt)) {
                LOG_WARN("Request failed and will not be retried - Attempt: " + std::to_string(attempt + 1) + "/" + std::to_string(max_retries + 1) + ", Error: " + response.get_error());
                return response;
            }
            attempt++;
            
            if (attempt > max_retries) {
                LOG_ERROR("Request failed after all " + std::to_string(max_retries + 1) + " attempts. Final error: " + last_response.get_error());
                break;
            }
            
            int delay_ms = calculate_backoff_delay(attempt);
            LOG_INFO("Request failed, retrying in " + std::to_string(delay_ms) + "ms (attempt " + std::to_string(attempt + 1) + "/" + std::to_string(max_retries + 1) + ") - Error: " + response.get_error() + " (Type: " + std::to_string(static_cast<int>(response.get_error_type())) + ")");
            
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        
        return last_response;
    }

private:
    std::shared_ptr<IHttpClient> inner_http_client;
    int max_retries;
    int base_backoff_ms;
    
    bool should_retry(const HTTPResponse& response, int attempt) const 
    {
        if (attempt >= max_retries) 
        {
            return false;
        }
        
        // Don't retry client errors (4xx) except for specific cases
        if (response.is_client_error()) {
            int status_code = response.get_status_code();
            return status_code == 408 || status_code == 429; // Request Timeout, Too Many Requests
        }
        
        // Retry server errors (5xx) and network errors
        return response.should_retry();
    }
    
    int calculate_backoff_delay(int attempt) const 
    {
        int delay_ms = base_backoff_ms * attempt;
        const int max_delay_ms = 60000; // 60 seconds
        return std::min(delay_ms, max_delay_ms);
    }
}; 