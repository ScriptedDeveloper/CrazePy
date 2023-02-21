#pragma once
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
using AnyVar = std::variant<std::string, int, bool, double, float, char>;
using ArgVector = std::vector<AnyVar>;
using VarMap = std::unordered_map<std::string, AnyVar>;


