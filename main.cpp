#include <store.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <thread>
#include <server.h>
#include <utils.h>

enum class Mode {
    CLI,
    SERVER
};

enum class cliArgOptions {
    MODE
};

void cli(Store store);

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

    Store store;
    store.load_from_log();

    if (mode == Mode::SERVER) {
        ServerOptions options;
        options.load_options();
        
        server(store, options);
    } else {
        cli(store);
    }

    return 0;
}

void cli(Store store){
    std::cout << "Starting Memstore in CLI mode.." << std::endl;

        std::string cmd, key, type;
        while (1) {
            std::cout << "> ";
            std::cin >> cmd;
            std::string cmd_lower = getlowercase(cmd);

            if (cmd_lower == "set") {
                std::string raw_value;
                Value value;
                std::cin >> key >> type;
                type = getlowercase(type);

                std::cin >> std::ws;

                // check for multi-word string value
                if(type == "string" && std::cin.peek() == '"'){
                    // consume double quotes
                    std::cin.get();
                    std::string str_val;
                    getline(std::cin >> std::ws, str_val, '"');

                    value = Value("\"" + str_val + "\"");
                    store.set(key, value);
                    continue;
                }
                std::cin >> raw_value;

                if (type == "int") value = Value(std::stoi(raw_value));
                else if (type == "double") value = Value(std::stod(raw_value));
                else if (type == "bool") {
                    const bool truthy = true;
                    if (raw_value == "true" || raw_value == "1") value = Value(truthy);
                    else if (raw_value == "false" || raw_value == "0") value = Value(!truthy);
                    else {
                        std::cout << "Invalid boolean value. Use 'true' or '1' for true, 'false' or '0' for false." << std::endl;
                        continue;
                    }
                }
                else if (type == "string") value = Value(raw_value);
                else {
                    std::cout << "Invalid type. Try again." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                store.set(key, value);
            }
            else if (cmd_lower == "get") {
                std::cin >> key;
                store.get(key);
            } else if (cmd_lower == "del") {
                std::cin >> key;
                store.del(key);
            } else if (cmd_lower == "exit") {
                std::cout << "$> Exiting..." << std::endl;
                break;
            } else if (cmd_lower == "help") {
                std::cout << "Memstore - An in-memory key-value store" << std::endl;
                std::cout << "Available commands: " << std::endl;
                
                // SET description
                std::cout << "> SET" << std::endl << "\t> -Add new value or update existing one" << std::endl;
                std::cout << "\t> -Syntax: SET <key_name> <value_type> <value>" << std::endl;
                std::cout << "\t> -Types available: INT, LONG, DOUBLE, BOOL, STRING" << std::endl;
                std::cout << "\t> -In case of multi-word string values, enclose them in double quotes\"\"" << std::endl;

                // GET description
                std::cout << "> GET" << std::endl << "\t> -Display value of existing key on the console." << std::endl;
                std::cout << "\t -Syntax: GET <key_name>" << std::endl;

                // DEL description
                std::cout << "> DEL" << std::endl << "\r> -Delete existing key-value pair." << std::endl;
                std::cout << "\t> -Syntax: DEL <key_name>" << std::endl;
            } else {
                std::cout << "Unknown command. Type HELP to get list of available commands" << std::endl;
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
}