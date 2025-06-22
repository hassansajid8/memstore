#include "store.hpp"
#include <iostream>
#include <fstream>
#include <string.h>

enum class Mode {
    CLI,
    SERVER
};

enum class cliArgOptions {
    MODE
};

void cli();

int main(int argc, char *argv[]) {
    Mode mode = Mode::CLI;
    cliArgOptions argOpts;

    for(int i=1;i<argc;i++){
        if(strcmp(argv[i], "-m") == 0) argOpts = cliArgOptions::MODE;
        else{
            if(argOpts == cliArgOptions::MODE){
                if (strcmp(argv[i], "cli") == 0) mode = Mode::CLI;
                else if(strcmp(argv[i], "server") == 0) mode = Mode::SERVER;
            }
        }
    }

    if (mode == Mode::SERVER) {
        std::cout << "Server mode coming soon..." << std::endl;
        exit(0);
    } else {
        cli();
    }

    return 0;
}

void cli(){
    std::cout << "Starting CLI mode.." << std::endl;

        Store store;
        store.load_from_log();

        std::string cmd, key, type;
        while (1) {
            std::cout << "> ";
            std::cin >> cmd;

            if (cmd == "SET" || cmd == "set") {
                std::string raw_value;
                Value value;
                std::cin >> key >> type >> raw_value;

                if (type == "INT" || type == "int") value = Value(std::stoi(raw_value));
                else if (type == "DOUBLE" || type == "double") value = Value(std::stod(raw_value));
                else if (type == "BOOL" || type == "bool") {
                    const bool truthy = true;
                    if (raw_value == "true" || raw_value == "1") value = Value(truthy);
                    else if (raw_value == "false" || raw_value == "0") value = Value(!truthy);
                    else {
                        std::cout << "Invalid boolean value. Use 'true' or '1' for true, 'false' or '0' for false." << std::endl;
                        continue;
                    }
                }
                else if (type == "STRING" || type == "string") value = Value(raw_value);
                else {
                    std::cout << "Invalid type. Try again." << std::endl;
                    continue;
                }

                store.set(key, value);
            }
            else if (cmd == "GET" || cmd == "get") {
                std::cin >> key;
                store.get(key);
            } else if (cmd == "DEL" || cmd == "del") {
                std::cin >> key;
                store.del(key);
            } else if (cmd == "EXIT" || cmd == "exit") {
                std::cout << "$> Exiting..." << std::endl;
                break;
            } else if (cmd == "HELP" || cmd == "help") {
                std::cout << "Stash - An in-memory key-value store" << std::endl;
                std::cout << "Available commands: " << std::endl;
                std::cout << "SET, GET, DEL, EXIT, HELP (Case-insensitive)" << std::endl;
            } else {
                std::cout << "Unknown command. Type HELP to get list of available commands" << std::endl;
            }
        }
}