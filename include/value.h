#pragma once
#include <string>

enum class ValueType{
    STRING,
    INT,
    LONG,
    DOUBLE,
    BOOL,
    NONE
};

union ValueData {
    int int_val;
    double double_val;
    bool bool_val;
    std::string* string_val;

    ValueData() {};
    ~ValueData() {};
};

class Value{
    private:
        ValueType val_type;
        ValueData val_data;

        void clear();

    public:
        Value();
        Value(int);
        Value(double);
        Value(bool);
        Value(const std::string&);
        Value(const Value&);
        Value& operator=(const Value&);
        ~Value();

        ValueType get_type() const;
        std::string to_string() const;

};
