#include <server.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>
#include <signal.h>
#include <unordered_map>
#include <store.h>
#include <fstream>
#include <crypto.h>
#include <utils.h>

std::string getRequestMethod(std::string req);
std::string getRoute(std::string req);
std::unordered_map<std::string, std::string> getHeaders(std::string req);
std::unordered_map<std::string, std::string> getParams(std::string route);
std::string getAuthKey();

void printHeaders(std::unordered_map<std::string, std::string>);


void ServerOptions::load_options() {
    std::ifstream conf("server.conf");
    
    // default options
    port = 8080;
    allowed_origins = "*/*";
    authorize = false;
    encrypt = false;
    
    // parse server.config
    if(!conf.is_open()){
        std::cout << "$> server.conf file not found. Default options will be used." << std::endl;
    } else {
        std::string option_name, value;

        while(conf >> option_name >> value){
            if(option_name == "PORT") port = std::stoi(value);
            else if (option_name == "ALLOWED_ORIGINS")allowed_origins = value;
            else if (option_name == "AUTHORIZE"){
                authorize = (value == "TRUE" || value == "1") ? true : false;
                auth_key = getAuthKey();                
            } else if(option_name == "ENCRYPTION"){
                encrypt = (value == "TRUE" || value == "1") ? true : false;
            }
        }
    }

    // log server options
    std::cout << "$> Server configured with options: " << std::endl;
    std::cout << "\tPORT: " << port << std::endl;
    std::cout << "\tAllowed Origins: " << allowed_origins << std::endl;
    std::cout << "\tAuthentication: " << authorize << std::endl;
    std::cout << "\tEncryption: " << encrypt << std::endl;
}


void server(Store store, ServerOptions options){

    // setup encryption 
    // TEA tea;

    // setup variables
    int sockfd, new_sockfd, pid;
    struct sockaddr_in address;
    long valread;
    int addrlen = sizeof(address);

    // create server socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("$> Error creating socket.");
        exit(EXIT_FAILURE);
    }

    // set socket to reuse address // only for development 
    int optval = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("$> Error setting socket options.");
        exit(EXIT_FAILURE);
    }
    
    // setup server address
    address.sin_family = INADDR_ANY;
    address.sin_port = htons(options.port);
    address.sin_addr.s_addr = INADDR_ANY;

    // bind socket to server address
    if(bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("$> Error binding socket.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // listen on socket
    if(listen(sockfd, 10) < 0){
        perror("$> Error listening.");
        exit(EXIT_FAILURE);
    }

    // accept connections loop
    while(1){
        std::cout << "Waiting for new connection..." << std::endl;

        // accept incoming connection
        if((new_sockfd = accept(sockfd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
            perror("$> Error accepting connection.");
            exit(EXIT_FAILURE);
        }

        std::cout << "$> Recieved new connection." << std::endl;

        // create child process to handle multiple requests at once
        pid = fork();
        if(pid < 0){
            perror("Error creating child process (fork()).");
            exit(EXIT_FAILURE);
        }

        if(pid == 0){ // in child process
            char buffer[30000] = {0};
            std::string json, msg;
            size_t bytes_sent;

            ssize_t valread = recv(new_sockfd, buffer, sizeof(buffer), 0);
            if(valread < 0){
                perror("$> Error in recv.");
                exit(EXIT_FAILURE);
            }

            std::string request(buffer, valread);
            std::cout << request << std::endl;
            
            std::unordered_map<std::string, std::string> headers = getHeaders(request);

            if(options.authorize){
                if(headers.find("Authorization") == headers.end()){
                    std::cout << "Unauthorized request recieved." << std::endl;
                    std::cout << "Host: " << headers["Host"] << std::endl;

                    json = R"({ "status": 403, "message": "Unauthorized"})";
                    msg = 
                        "HTTP/1.1 403 Unauthorized\r\n"
                        "Content-type: application/json\r\n"
                        "Content-length: " + std::to_string(json.size()) + "\r\n"
                        "WWW-Authenticate: Basic\r\n"
                        "\r\n" +
                        json;

                    bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                    std::cout << "Bytes sent: " << bytes_sent << std::endl;

                    close(new_sockfd);
                    continue;
                } else {
                    // verify authentication
                    std::string auth_mode = getlowercase(headers["Authorization"].substr(0, 5));
                    std::string recieved_key = headers["Authorization"].substr(6);

                    if(auth_mode != "basic"){
                        json = R"({ "status": 403, "message": "Unauthorized"})";
                        msg = 
                            "HTTP/1.1 403 Unauthorized\r\n"
                            "Content-type: application/json\r\n"
                            "Content-length: " + std::to_string(json.size()) + "\r\n"
                            "WWW-Authenticate: Basic\r\n"
                            "\r\n" +
                            json;

                        bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                        std::cout << "Bytes sent: " << bytes_sent << std::endl;

                        close(new_sockfd);
                        continue;
                    }

                    if(recieved_key != options.auth_key){
                        json = R"({ "status": 403, "message": "Unauthorized"})";
                        msg = 
                            "HTTP/1.1 403 Unauthorized\r\n"
                            "Content-type: application/json\r\n"
                            "Content-length: " + std::to_string(json.size()) + "\r\n"
                            "WWW-Authenticate: Basic\r\n"
                            "\r\n" +
                            json;

                        bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                        std::cout << "Bytes sent: " << bytes_sent << std::endl;

                        close(new_sockfd);
                        continue;
                    }
                }
            }

            // get method
            std::string method = getRequestMethod(request);
            std::cout << "Method = " << method << std::endl;

            if(method == "GET"){
                std::string route = getRoute(request);
                std::cout << "Route = " << route << std::endl;

                // default route
                if(route == "/"){
                    json = R"( {
                    "project": "Memstore",
                    "description": "Simple in-memory key-value database written in C++",
                    "routes": {
                        "/get": "Method: GET. Returns value to a given key. Key is passed as query parameter.",
                        "/set": "Method: POST. Creates new or updates existing entry.",
                        "/del": "Method: GET. Deletes an entry. Key is passed as query parameter."
                    }                    
                    } )";
                    msg = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-type: application/json\r\n"
                        "Content-length:" + std::to_string(json.size()) + "\r\n"
                        "\r\n"
                        + json;

                    bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                    std::cout << "Bytes sent = " << bytes_sent << std::endl;

                }
                // get route
                else if(route.rfind("/get", 0) == 0){
                    std::unordered_map<std::string, std::string> params = getParams(route);

                    if(params.count("key") == 0){
                        // return error
                    }
                    else {
                        json = R"( {"value": "some_value"} )";
                        msg = 
                            "HTTP/1.1 200 OK\r\n"
                            "Content-type: application/json\r\n"
                            "Content-length:" + std::to_string(json.size()) + "\r\n"
                            "\r\n"
                            + json;
    
                        bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                        std::cout << "Bytes sent = " << bytes_sent << std::endl;
                    }

                } 
                else {
                    msg = "HTTP/1.1 404 Not Found\r\n";
                    bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                    std::cout << "Bytes sent = " << bytes_sent << std::endl;                    
                }
            }
            else if(method == "POST"){
                json = R"({ method: POST })";
                msg = 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-type: application/json\r\n"
                    "Content-length:" + std::to_string(json.size()) + "\r\n"
                    "\r\n"
                    + json;
            }
            close(new_sockfd);
        } else {
            close(new_sockfd);
        }
    }
    close(sockfd);
    return;
}

std::string getRequestMethod(std::string req){
    int i = 0;
    std::string method;
    while(req[i] != ' '){
        method += req[i];
        i++;
    }

    return method;
}

std::string getRoute(std::string req){
    int i;
    std::string route;

    for(i=0;req[i]!=' ';i++);
    i++;

    while(req[i] != ' '){
        route += req[i];
        i++;
    }

    return route;
}

std::unordered_map<std::string, std::string> getHeaders(std::string req) {
    std::unordered_map<std::string, std::string> map;
    std::istringstream stream(req);
    std::string line;

    // skip the request line (GET / HTTP/1.1)
    std::getline(stream, line);

    while (std::getline(stream, line)) {
        if (line == "\r" || line == "") break; // End of headers

        size_t pos = line.find(':');
        if (pos == std::string::npos) continue; // malformed line

        std::string name = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // trim whitespace
        name.erase(0, name.find_first_not_of(" \t\r"));
        name.erase(name.find_last_not_of(" \t\r") + 1);
        value.erase(0, value.find_first_not_of(" \t\r"));
        value.erase(value.find_last_not_of(" \t\r") + 1);

        map[name] = value;
    }

    return map;
}

std::unordered_map<std::string, std::string> getParams(std::string route) {
    std::unordered_map<std::string, std::string> params;
    size_t i = 0;

    while (route[i] && route[i] != '?') i++;

    if (!route[i]) return params;
    i++;

    while (route[i]) {
        std::string name;
        std::string value;

        while (route[i] && route[i] != '=') {
            name += route[i++];
        }

        if (route[i] == '=') i++;

        while (route[i] && route[i] != '&') {
            value += route[i++];
        }

        if (route[i] == '&') i++;

        params[name] = value;
    }

    return params;
}

std::string getAuthKey(){
    std::string name, value;
    bool keyfound = false;
    std::ifstream envfile(".env");

    if(!envfile.is_open()){
        perror(".env file not found. Aborting.");
        exit(EXIT_FAILURE);
    }

    while(envfile >> name >> value){
        if(name == "AUTH_KEY"){
            keyfound = true;
            return value;
        }
    }

    if(!keyfound){
        perror("AUTH_KEY not found in .env. Aborting.");
        exit(EXIT_FAILURE);
    }
}

void printHeaders(std::unordered_map<std::string, std::string> map){
    for(auto it = map.begin();it != map.end(); it++){
        std::cout << "$> " << it->first << ": " << it->second << std::endl;
    }
}
