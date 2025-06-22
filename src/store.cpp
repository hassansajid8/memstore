#include "store.hpp"
#include <fstream>
#include <iostream>
#include <string>

void Store::set(std::string key, Value value){
    map[key] = value;
    append_to_log(Operation::SET, key, value);
}

void Store::get(std::string key){
    if(map.find(key) != map.end()){
        std::cout << map[key].to_string() << std::endl;
    } else {
        std::cout << "$> Key not found" << std::endl;
    }
}

void Store::del(std::string key){
    map.erase(key);
    append_to_log(Operation::DEL, key, "NULL");
}

void Store::append_to_log(Operation op, std::string key, const Value value){
    std::ofstream log("data.log", std::ios::app);
    if(!log.is_open()){
        std::cerr << "$> Failed to open file" << std::endl;
        return;
    }

    std::string type_str;
    switch(value.get_type()){
        case ValueType::INT:
            type_str = "INT";
            break;
        case ValueType::DOUBLE:
            type_str = "DOUBLE";
            break;
        case ValueType::BOOL:
            type_str = "BOOL";
            break;
        case ValueType::STRING:
            type_str = "STRING";
            break;
    }

    if(op == Operation::SET){
        log << "SET " << key << " " << type_str << " " << value.to_string() << std::endl;
    }
    else if(op == Operation::DEL){
        log << "DEL " << key << " " << type_str << " " << value.to_string() << std::endl;
    }

    log.close();
}

void Store::load_from_log() {
    std::ifstream log("data.log");
    if(!log.is_open()){
        std::cerr << "$> No log file found. New file will be created." << std::endl;
        return;
    }

    std::string op, key, type;

    while(log >> op >> key >> type){
        if(op == "SET"){
            std::string raw_value;
            Value value;
            std::getline(log >> std::ws, raw_value);

            if(type == "INT") value = Value(std::stoi(raw_value));
            else if(type == "DOUBLE") value = Value(std::stod(raw_value));
            else if(type == "BOOL") {
                if(raw_value == "true" || "1") value = Value(true);
                else if(raw_value == "false" || "0") value = Value(false);
                else {
                    std::cout << "Invalid boolean value for key " << key << ". Skipping." << std::endl;
                    continue;
                }
            }
            else if(type == "STRING") value = Value(raw_value);
            else {
                std::cout << "$> Unknown type for key " << key << ". Skipping." << std::endl;
                continue;
            }
            
            map[key] = value;
        } else if (op == "DELETE") {
            map.erase(key);
        }
    }

    log.close();
}