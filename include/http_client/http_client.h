#pragma once
#include <string>
#include <map>
#include <optional>
#include <stdexcept>
#include "utils/logger.h"

enum class HttpMethod { GET, POST, PUT, DELETE };

enum class HttpErrorType 
{
    NONE,
    INVALID_URL,
    INVALID_METHOD,
    CONNECTION_TIMEOUT,
    REQUEST_TIMEOUT,
    NETWORK_ERROR,
    CONNECTION_REFUSED,
    INVALID_REQUEST,
    UNKNOWN_ERROR
};

class HTTPRequest 
{
public:
    HTTPRequest() = default;
    HTTPRequest(
        HttpMethod _method, 
        const std::string& _url, 
        const std::map<std::string, std::string>& _headers = {}, 
        const std::string& _body = "",
        int _connect_timeout_ms = 2000,
        int _total_timeout_ms = 5000)
        : method(_method), url(_url), headers(_headers), body(_body), connect_timeout_ms(_connect_timeout_ms), total_timeout_ms(_total_timeout_ms) {}

    //Getters
    HttpMethod get_method() const { return method; }
    const std::string& get_url() const { return url; }
    const std::map<std::string, std::string>& get_headers() const { return headers; }
    const std::string& get_body() const { return body; }
    int get_connect_timeout_ms() const { return connect_timeout_ms; }
    int get_total_timeout_ms() const { return total_timeout_ms; }

    //Setters:
    void set_method(HttpMethod _method) { method = _method; }
    void set_url(const std::string& _url) { url = _url; }
    void set_headers(const std::map<std::string, std::string>& _headers) { headers = _headers; }
    void set_body(const std::string& _body) { body = _body; }
    void set_connect_timeout_ms(int ms) { connect_timeout_ms = ms; }
    void set_total_timeout_ms(int ms) { total_timeout_ms = ms; }

    //Validation methods
    bool is_valid() const;
    std::string get_validation_error() const;

private:
    HttpMethod method;
    std::string url;
    std::map<std::string, std::string> headers;
    std::string body;
    int connect_timeout_ms = 2000;
    int total_timeout_ms = 5000;
};

class HTTPResponse 
{
public:
    //Constructors
    HTTPResponse() = default;
    HTTPResponse(
        int _status_code, 
        const std::string& _body, 
        const std::map<std::string, std::string>& _headers = {}, 
        const std::string& _error = "",
        HttpErrorType _error_type = HttpErrorType::NONE)
        : status_code(_status_code), body(_body), headers(_headers), error(_error), error_type(_error_type) {}

    //Getters
    int get_status_code() const { return status_code; }
    const std::string& get_body() const { return body; }
    const std::map<std::string, std::string>& get_headers() const { return headers; }
    const std::string& get_error() const { return error; }
    HttpErrorType get_error_type() const { return error_type; }

    //Setters
    void set_status_code(int _status_code) { status_code = _status_code; }
    void set_body(const std::string& _body) { body = _body; }
    void set_headers(const std::map<std::string, std::string>& _headers) { headers = _headers; }
    void set_error(const std::string& _error) { error = _error; }
    void set_error_type(HttpErrorType _error_type) { error_type = _error_type; }

    //Utility methods
    bool is_success() const { return error_type == HttpErrorType::NONE && status_code >= 200 && status_code < 300; }
    bool is_client_error() const { return status_code >= 400 && status_code < 500; }
    bool is_server_error() const { return status_code >= 500 && status_code < 600; }
    bool is_network_error() const { return error_type != HttpErrorType::NONE; }
    bool should_retry() const { return is_network_error() || is_server_error(); }

private:
    int status_code = 0;
    std::string body;
    std::map<std::string, std::string> headers;
    std::string error;
    HttpErrorType error_type = HttpErrorType::NONE;
};

class IHttpClient 
{
public:
    virtual ~IHttpClient() = default;
    virtual HTTPResponse request(const HTTPRequest& request) const = 0;
};

bool validate_url_format(const std::string& url);


class HTTPClient : public IHttpClient 
{
public:
    HTTPResponse request(const HTTPRequest& request) const override;

private:
    HttpErrorType categorize_cpr_error(const std::string& cpr_error) const;
    std::string sanitize_url(const std::string& url) const;
};

