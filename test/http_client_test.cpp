#include <catch2/catch_test_macros.hpp>
#include "http_client/http_client.h"
#include <memory>

TEST_CASE("HTTPRequest validation", "[http_client]") {
    SECTION("Valid request") {
        HTTPRequest request(HttpMethod::GET, "https://httpbin.org/get");
        REQUIRE(request.is_valid());
        REQUIRE(request.get_validation_error().empty());
    }
    
    SECTION("Empty URL") {
        HTTPRequest request(HttpMethod::GET, "");
        REQUIRE_FALSE(request.is_valid());
        REQUIRE(request.get_validation_error() == "URL cannot be empty");
    }
    
    SECTION("Invalid URL format") {
        HTTPRequest request(HttpMethod::GET, "not-a-url");
        REQUIRE_FALSE(request.is_valid());
        REQUIRE(request.get_validation_error() == "Invalid URL format");
    }
    
    SECTION("Invalid timeout values") {
        HTTPRequest request(HttpMethod::GET, "https://httpbin.org/get", {}, "", 0, 5000);
        REQUIRE_FALSE(request.is_valid());
        REQUIRE(request.get_validation_error() == "Connect timeout must be greater than 0");
        
        HTTPRequest request2(HttpMethod::GET, "https://httpbin.org/get", {}, "", 2000, 0);
        REQUIRE_FALSE(request2.is_valid());
        REQUIRE(request2.get_validation_error() == "Total timeout must be greater than 0");
    }
    
    SECTION("Connect timeout greater than total timeout") {
        HTTPRequest request(HttpMethod::GET, "https://httpbin.org/get", {}, "", 5000, 2000);
        REQUIRE_FALSE(request.is_valid());
        REQUIRE(request.get_validation_error() == "Connect timeout cannot be greater than total timeout");
    }
}

TEST_CASE("URL validation", "[http_client]") {
    SECTION("Valid URLs") {
        REQUIRE(validate_url_format("https://httpbin.org/get"));
        REQUIRE(validate_url_format("http://localhost:8080/api"));
        REQUIRE(validate_url_format("https://api.example.com/v1/users"));
    }
    
    SECTION("Invalid URLs") {
        REQUIRE_FALSE(validate_url_format(""));
        REQUIRE_FALSE(validate_url_format("not-a-url"));
        REQUIRE_FALSE(validate_url_format("ftp://example.com"));
        REQUIRE_FALSE(validate_url_format("https://"));
    }
}

TEST_CASE("HTTPResponse utility methods", "[http_response]") {
    SECTION("Success responses") {
        HTTPResponse response(200, "OK", {}, "", HttpErrorType::NONE);
        REQUIRE(response.is_success());
        REQUIRE_FALSE(response.is_client_error());
        REQUIRE_FALSE(response.is_server_error());
        REQUIRE_FALSE(response.is_network_error());
        REQUIRE_FALSE(response.should_retry());
    }
    
    SECTION("Client error responses") {
        HTTPResponse response(404, "Not Found", {}, "", HttpErrorType::NONE);
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.is_client_error());
        REQUIRE_FALSE(response.is_server_error());
        REQUIRE_FALSE(response.is_network_error());
        REQUIRE_FALSE(response.should_retry());
    }
    
    SECTION("Server error responses") {
        HTTPResponse response(500, "Internal Server Error", {}, "", HttpErrorType::NONE);
        REQUIRE_FALSE(response.is_success());
        REQUIRE_FALSE(response.is_client_error());
        REQUIRE(response.is_server_error());
        REQUIRE_FALSE(response.is_network_error());
        REQUIRE(response.should_retry());
    }
    
    SECTION("Network error responses") {
        HTTPResponse response(0, "", {}, "Connection failed", HttpErrorType::NETWORK_ERROR);
        REQUIRE_FALSE(response.is_success());
        REQUIRE_FALSE(response.is_client_error());
        REQUIRE_FALSE(response.is_server_error());
        REQUIRE(response.is_network_error());
        REQUIRE(response.should_retry());
    }
}

TEST_CASE("HTTPClient request with invalid input", "[http_client]") {
    auto client = std::make_shared<HTTPClient>();
    
    SECTION("Invalid request should return error") {
        HTTPRequest request(HttpMethod::GET, "");
        HTTPResponse response = client->request(request);
        
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.get_error_type() == HttpErrorType::INVALID_REQUEST);
        REQUIRE(response.get_error().find("URL cannot be empty") != std::string::npos);
    }
    
    SECTION("Invalid URL format should return error") {
        HTTPRequest request(HttpMethod::GET, "not-a-url");
        HTTPResponse response = client->request(request);
        
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.get_error_type() == HttpErrorType::INVALID_REQUEST);
    }
    
    SECTION("Invalid HTTP method should return error") {
        HTTPRequest request(static_cast<HttpMethod>(999), "https://httpbin.org/get");
        HTTPResponse response = client->request(request);
        
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.get_error_type() == HttpErrorType::INVALID_METHOD);
    }
}

TEST_CASE("HTTPClient successful request", "[http_client]") {
    auto client = std::make_shared<HTTPClient>();
    
    SECTION("GET request to working endpoint") {
        HTTPRequest request(HttpMethod::GET, "https://httpbin.org/get");
        HTTPResponse response = client->request(request);
        
        // This test might fail if network is down, so we check for either success or network error
        if (response.is_success()) {
            REQUIRE(response.get_status_code() == 200);
            REQUIRE(response.get_body().find("httpbin.org") != std::string::npos);
        } else {
            // If network is down, we should get a network error
            REQUIRE(response.is_network_error());
        }
    }
}

TEST_CASE("HTTPClient error categorization", "[http_client]") {
    auto client = std::make_shared<HTTPClient>();
    
    SECTION("Connection timeout error") {
        HTTPRequest request(HttpMethod::GET, "http://localhost:9999/timeout");
        HTTPResponse response = client->request(request);
        
        // Should get a network error (connection refused or timeout)
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.is_network_error());
    }
}
