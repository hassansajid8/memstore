#include <utils.h>

std::string getlowercase(std::string str){
    std::string retval;
    for(char c : str){
        retval += std::tolower(c);
    }

    return retval;
}

