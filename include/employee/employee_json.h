#pragma once
#include "employee.h"
#include <nlohmann/json.hpp>

void to_json(nlohmann::json& j, const Employee& employee);
void from_json(const nlohmann::json& j, Employee& employee);
void to_json_create_employee(nlohmann::json& j, const Employee& employee);
//void from_json_create_employee(const nlohmann::json& j, Employee& employee);