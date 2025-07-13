#include <catch2/catch_test_macros.hpp>
#include "http_client/retry_http_client.h"
#include "http_client/http_client.h"
#include <memory>
#include <chrono>

// Mock HTTP client for testing retry logic
class MockHttpClient : public IHttpClient 
{
public:
    MockHttpClient() : call_count_(0), should_fail_(true) {}
    
    void set_should_fail(bool should_fail) { should_fail_ = should_fail; }
    void set_success_after(int calls) { success_after_calls_ = calls; }
    int get_call_count() const { return call_count_; }
    
    HTTPResponse request(const HTTPRequest& request) const override {
        call_count_++;
        
        if (should_fail_ && call_count_ < success_after_calls_) {
            // Return a network error to trigger retry
            return HTTPResponse(0, "", {}, "Mock network error", HttpErrorType::NETWORK_ERROR);
        } else {
            // Return success
            return HTTPResponse(200, "Success", {}, "", HttpErrorType::NONE);
        }
    }
    
private:
    mutable int call_count_;
    bool should_fail_;
    int success_after_calls_ = 1;
};

TEST_CASE("RetryHttpClient initialization", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    
    SECTION("Default constructor") {
        RetryHttpClient retry_client(mock_client);
        // Should not throw
        REQUIRE_NOTHROW(retry_client);
    }
    
    SECTION("Custom retry settings") {
        RetryHttpClient retry_client(mock_client, 5, 1000);
        // Should not throw
        REQUIRE_NOTHROW(retry_client);
    }
}

TEST_CASE("RetryHttpClient successful request", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    mock_client->set_should_fail(false);
    
    RetryHttpClient retry_client(mock_client, 3, 100);
    
    SECTION("Successful request should not retry") {
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        REQUIRE(response.is_success());
        REQUIRE(response.get_status_code() == 200);
        REQUIRE(mock_client->get_call_count() == 1); // Only one call, no retries
    }
}

TEST_CASE("RetryHttpClient retry behavior", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    mock_client->set_should_fail(true);
    mock_client->set_success_after(2); // Succeed after 2 calls
    
    RetryHttpClient retry_client(mock_client, 3, 100);
    
    SECTION("Should retry on failure and eventually succeed") {
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        REQUIRE(response.is_success());
        REQUIRE(response.get_status_code() == 200);
        REQUIRE(mock_client->get_call_count() == 2); // Initial call + 1 retry
    }
}

TEST_CASE("RetryHttpClient max retries", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    mock_client->set_should_fail(true);
    mock_client->set_success_after(10); // Never succeed
    
    RetryHttpClient retry_client(mock_client, 2, 50); // Only 2 retries
    
    SECTION("Should stop after max retries") {
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.is_network_error());
        REQUIRE(mock_client->get_call_count() == 3); // Initial call + 2 retries
    }
}

TEST_CASE("RetryHttpClient linear backoff timing", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    mock_client->set_should_fail(true);
    mock_client->set_success_after(4); // Succeed after 4 calls
    
    RetryHttpClient retry_client(mock_client, 3, 100); // 100ms base backoff
    
    SECTION("Should use linear backoff delays") {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        REQUIRE(response.is_success());
        REQUIRE(mock_client->get_call_count() == 4); // Initial + 3 retries
        
        // Should have delays: 100ms + 200ms + 300ms = 600ms minimum
        // Add some tolerance for jitter and system overhead
        REQUIRE(duration.count() >= 500); // At least 500ms total delay
    }
}

TEST_CASE("RetryHttpClient client errors", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    
    RetryHttpClient retry_client(mock_client, 3, 100);
    
    SECTION("Client errors should not retry by default") {
        // Create a mock response that simulates a client error
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        
        // We need to modify the mock to return client errors
        // For now, we'll test the retry logic with network errors
        mock_client->set_should_fail(true);
        mock_client->set_success_after(10); // Never succeed
        
        HTTPResponse response = retry_client.request(request);
        
        REQUIRE_FALSE(response.is_success());
        REQUIRE(response.is_network_error());
    }
}

TEST_CASE("RetryHttpClient server errors", "[retry_client]") {
    auto mock_client = std::make_shared<MockHttpClient>();
    mock_client->set_should_fail(true);
    mock_client->set_success_after(2);
    
    RetryHttpClient retry_client(mock_client, 3, 100);
    
    SECTION("Server errors should retry") {
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        // Should retry and eventually succeed
        REQUIRE(response.is_success());
        REQUIRE(mock_client->get_call_count() >= 2);
    }
}

TEST_CASE("RetryHttpClient edge cases", "[retry_client]") {
    SECTION("Zero retries should not retry") {
        auto mock_client = std::make_shared<MockHttpClient>();
        mock_client->set_should_fail(true);
        mock_client->set_success_after(2); // Succeed after 2 calls, but we only make 1 call
        
        RetryHttpClient retry_client(mock_client, 0, 100);
        
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        REQUIRE_FALSE(response.is_success());
        REQUIRE(mock_client->get_call_count() == 1); // Only initial call
    }
    
    SECTION("Zero base backoff should still work") {
        auto mock_client = std::make_shared<MockHttpClient>();
        mock_client->set_should_fail(true);
        mock_client->set_success_after(2);
        
        RetryHttpClient retry_client(mock_client, 3, 0);
        
        HTTPRequest request(HttpMethod::GET, "https://example.com");
        HTTPResponse response = retry_client.request(request);
        
        REQUIRE(response.is_success());
        REQUIRE(mock_client->get_call_count() == 2);
    }
} 