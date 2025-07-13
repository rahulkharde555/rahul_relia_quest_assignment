# Assignment Details:

Assignment is divided into two main modules internally: 
    A) Rest API endpoints implemented usinig CROW framework: (All the endpoints are mentioned below) 
    B) libemployee_api.a : library which calls the mock apis, and provides the business logic for all the mentionied endpoints.

Quick Start
    Prerequisites
        - C++17 compatible compiler 
        - CMake 3.15+
        - vcpkg package manager

    Installation
    
        1. Install Dependencies
            - $HOME/vcpkg/vcpkg install cpr crow nlohmann-json catch2

        2. Build the Project
            - mkdir build && cd build
            - cmake ..
            - make

        3. Run the Server
            -  ./employee_server

        4. Test the API
            - curl http://localhost:8111/api/v1/employees


A) Implemented the below API using CROW library:

    1. getAllEmployees: 

        URL: http://localhost:8111/api/v1/employees
        Method: GET
        Input: - 
        Output: - JSON response containing Employees array


    2. getEmployeeByID:

        URL: http://localhost:8111/api/v1/employees/<employee-id>
        Method: GET
        Input: employee-id in path
        Output: JSON Reponse containing the Employee details


    3. getEmployeeNameBySearch: 

        URL: http://localhost:8111/api/v1/employees/search/<name-fragment>
        Method: GET
        Input: name-fragment in path
        Output: JSON response containing the Employee details. 


    4. getHigestSalary: 

        URL: http://localhost:8111/api/v1/employees/highestsalary
        Method: GET
        Input: - 
        Output: JSON response containing the Higest salary. 


    5. getTop10HighestEarningEmployeeNames: 

        URL: http://localhost:8111/api/v1/employees/top-10-highest-earning
        Method: GET
        Input: - 
        Output: JSON response containing the top 10 employess based on salary. 


    6. createEmployee:

        URL: http://localhost:8111/api/v1/employees
        Method: POST
        Input: Employee Details in JSON format passed in Body. 
        Output: Created employee in JSON format. 


    7. deleteEmployeeById: 

        URL: http://localhost:8111/api/v1/employees/<employee-id>
        Method: DELETE
        Input: employee-id in path. 
        Output: Name of Deleted employee in JSON format. 


B) Implemented the libemployee_api.a library which exposes following functions: 

    1. ApiResponse getAllEmployees(std::vector<Employee>& employees) const;

    2. ApiResponse getEmployeeById(const std::string& id, Employee& employee) const;

    3. ApiResponse createEmployee(const Employee& employee, Employee& created_employee) const;

    4. ApiResponse deleteEmployeeById(const std::string& id, std::string& employee_name) const;

    5. ApiResponse getEmployeeByNameSearch(const std::string& name, std::vector<Employee>& employees) const;

    6. ApiResponse getTop10HighestEarningEmployeeNames(std::vector<std::string>& names) const;

    7. ApiResponse getHighestSalaryFromEmployees(int& highest_salary) const;

        Where, 
        ApiResponse is return type which specifies the success, failure, return code, message string. 


Steps to build: 

    1. Dependency: 
        This repo is dependent on following libraries: 

        - libcurl
        - libcpr (Wrapper around libcurl)
        - nlohmann_json (For handling the JSON data)
        - Catch2 
        - Crow 

    2. Compilation: 
        - mkdir build; cd build; 
        - cmake .. 
        - make 

    3. libraries and binaries details: 

        - libemployee.a
            Library containing the Business logic 

        - employee_api_tests
            Binary containing the tests for employee_api 

        - employee_server
            Binary containing the code for server and rest endpoints: 

        - example_usage
            Binary containing the examples to call the functions from libemployee_api

    4. installation of the dependencies: 
        - All the dependencies are iinstalled using the vcpkg package manager: 
        - command to install 
            - $HOME/vcpkg/vcpkg install cpr
            - $HOME/vcpkg/vcpkg install crow
            - $HOME/vcpkg/vcpkg install nlohmann-json
            - $HOME/vcpkg/vcpkg install catch2

        - steps to see if the installatios is complete 
            - $HOME/vcpkg/vcpkg list 
            - See if all the installed libraries are present. 

How to run: 

    1. Run the provided mock server. 
    2. run the employee_server by using following command: 
        - ./employee_server 
    3. Call the rest end points mentioned above. 