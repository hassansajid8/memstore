#include "value.hpp"

void Value::clear(){
    if(val_type == ValueType::STRING){
        delete(val_data.string_val);
        val_data.string_val = nullptr;
    }
    val_type = ValueType::NONE;
}


// constructor overloads for value type
Value::Value(){
    val_type = ValueType::NONE;
}

Value::Value(int v){
    val_type = ValueType::INT;
    val_data.int_val = v;
}

Value::Value(double v){
    val_type = ValueType::DOUBLE;
    val_data.double_val = v;
}

Value::Value(bool v){
    val_type = ValueType::BOOL;
    val_data.bool_val = v;
}

Value::Value(const std::string& v){
    val_type = ValueType::STRING;
    val_data.string_val = new std::string(v);
}

Value::Value(const Value& other){
    val_type = other.val_type;
    switch(val_type){
        case ValueType::INT:
            val_data.int_val = other.val_data.int_val;
            break;
        case ValueType::DOUBLE:
            val_data.double_val = other.val_data.double_val;
            break;
        case ValueType::BOOL:
            val_data.bool_val = other.val_data.bool_val;
            break;
        case ValueType::STRING:
            val_data.string_val = new std::string(*other.val_data.string_val);
            break;
    }
}

Value& Value::operator=(const Value& other){
    if(this == &other) return *this;

    clear();

    val_type = other.val_type;
    switch(val_type){
        case ValueType::INT:
            val_data.int_val = other.val_data.int_val;
            break;
        case ValueType::DOUBLE:
            val_data.double_val = other.val_data.double_val;
            break;
        case ValueType::BOOL:
            val_data.bool_val = other.val_data.bool_val;
            break;
        case ValueType::STRING:
            val_data.string_val = new std::string(*other.val_data.string_val);
            break;
    }

    return *this;
}

Value::~Value(){
    clear();
}

ValueType Value::get_type() const{
    return val_type;
}

std::string Value::to_string() const {
    switch(val_type){
        case ValueType::INT:
            return std::to_string(val_data.int_val);
            break;
        case ValueType::DOUBLE:
            return std::to_string(val_data.double_val);
            break;
        case ValueType::BOOL:
            return std::to_string(val_data.bool_val);
            break;
        case ValueType::STRING:
            return *val_data.string_val;
        default:
            return "<invalid>";
    }
}
