cmake_minimum_required(VERSION 3.19)

project(employee_api VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fix for fmt library compatibility issues
add_compile_definitions(FMT_HEADER_ONLY=1)

set(CMAKE_OSX_ARCHITECTURES "x86_64")

include(/Users/rahulkharde555/vcpkg/scripts/buildsystems/vcpkg.cmake)

# Find dependencies
find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Catch2 REQUIRED)
find_package(Crow REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/employee
    ${CMAKE_SOURCE_DIR}/include/http_client
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Source files for the employee library
set(EMPLOYEE_SOURCES 
    src/employee/employee.cpp 
    src/employee/employee_json.cpp 
    src/employee/employee_api.cpp 
    src/http_client/http_client.cpp 
    src/http_client/url_encode.cpp
    src/utils/logger.cpp
    src/utils/http_response_utils.cpp
    src/utils/validation_utils.cpp
)

# Main employee library
add_library(employee STATIC ${EMPLOYEE_SOURCES})
target_link_libraries(employee PRIVATE cpr::cpr nlohmann_json::nlohmann_json)

# Crow REST API server
add_executable(employee_server main_crow.cpp)
target_link_libraries(employee_server PRIVATE employee Crow::Crow nlohmann_json::nlohmann_json)

# Example usage executable
add_executable(example_usage examples/example_usage.cpp)
target_link_libraries(example_usage PRIVATE employee nlohmann_json::nlohmann_json)

# Testing
enable_testing()

# Test sources
set(TEST_SOURCES
    test/test_main.cpp
    test/http_client_test.cpp
    test/retry_client_test.cpp
    test/employee_test.cpp
    test/employee_api_test.cpp
)

# Main test executable
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE employee Catch2::Catch2WithMain)
target_link_libraries(unit_tests PRIVATE stdc++)

# Add tests
add_test(NAME UnitTests COMMAND unit_tests)

# Individual test targets for specific components

add_executable(http_client_tests test/test_main.cpp test/http_client_test.cpp)
target_link_libraries(http_client_tests PRIVATE employee Catch2::Catch2WithMain)
add_test(NAME HttpClientTests COMMAND http_client_tests)

add_executable(employee_tests test/test_main.cpp test/employee_test.cpp)
target_link_libraries(employee_tests PRIVATE employee Catch2::Catch2WithMain)
add_test(NAME EmployeeTests COMMAND employee_tests)

add_executable(employee_api_tests test/test_main.cpp test/employee_api_test.cpp)
target_link_libraries(employee_api_tests PRIVATE employee Catch2::Catch2WithMain)
add_test(NAME EmployeeApiTests COMMAND employee_api_tests)

