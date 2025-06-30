#pragma once
#include <unordered_map>
#include <iostream>
#include "value.h"

enum class Operation {
    SET = 1,
    DEL = 3
};

class Store {
private:
    std::unordered_map<std::string, Value> map;
public:    
    void load_from_log();
    void append_to_log(Operation op, std::string key, const Value value);

    void set(std::string key, Value value);
    void get(std::string key);
    void del(std::string key);
};
