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

void ServerOptions::load_options() {
    std::ifstream conf("server.conf");
    
    port = 8080;
    allowed_origins = "*/*";
    authenticate = false;
    
    // parse server.config
    if(!conf.is_open()){
        std::cout << "$> server.conf file not found. Default options will be used." << std::endl;
    } else {
        std::string option_name, value;

        while(conf >> option_name >> value){
            if(option_name == "PORT"){
                port = std::stoi(value);
            } else if (option_name == "ALLOWED_ORIGINS"){
                allowed_origins = value;
            } else if (option_name == "AUTHENTICATE"){
                if(value == "TRUE" || value == "1"){
                    authenticate = true;
                } else {
                    authenticate = false;
                }
            }
        }
    }
}

std::string getRequestMethod(const char *buf);
std::string getRoute(const char *buf);
std::unordered_map<std::string, std::string> getHeaders(const char *buf);

void printHeaders(std::unordered_map<std::string, std::string>);

void server(Store store, ServerOptions options){

    // setup encryption module
    TEA tea;

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

        std::cout << "recieved new connection." << std::endl;

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

            valread = read(new_sockfd, buffer, 30000);
            
            std::unordered_map<std::string, std::string> headers = getHeaders(buffer);

            if(options.authenticate){
                if(headers.find("Authorization") == headers.end()){
                    std::cout << "Unauthorized request recieved." << std::endl;
                    std::cout << "Origin: " << headers["Origin"] << std::endl;

                    json = R"({ "status": 403, "message": "Unauthorized"})";
                    msg = 
                        "HTTP/1.1 403 Unauthorized\r\n"
                        "Content-type: application/json\r\n"
                        "Content=length: " + std::to_string(sizeof(json)) + "\r\n"
                        "WWW-Authenticate: Basic\r\n"
                        "\r\n" +
                        json;

                    bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                    std::cout << "Bytes sent: " << bytes_sent << std::endl;

                    close(new_sockfd);
                    continue;
                } else {
                    // verify authentication
                }
            }

            // get method
            std::string method = getRequestMethod(buffer);
            std::cout << "Method = " << method << std::endl;

            if(method == "GET"){
                std::string route = getRoute(buffer);
                std::cout << "Route = " << route << std::endl;

                if(route == "/"){

                    json = R"({ "route": "/", "name": "Home page" })";
                    msg = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-type: application/json\r\n"
                        "Content-length:" + std::to_string(json.size()) + "\r\n"
                        "\r\n"
                        + json;

                    bytes_sent = send(new_sockfd, msg.c_str(), (size_t)msg.length(), 0);
                    std::cout << "Bytes sent = " << bytes_sent << std::endl;

                } else {
                    msg = "HTTP/1.1 403 Forbidden\r\n";
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

std::string getRequestMethod(const char *buf){
    int i = 0;
    std::string method;
    while(buf[i] != ' '){
        method += buf[i];
        i++;
    }

    return method;
}

std::string getRoute(const char *buf){
    int i;
    std::string route;

    for(i=0;buf[i]!=' ';i++);
    i++;

    while(buf[i] != ' '){
        route += buf[i];
        i++;
    }

    return route;
}

std::unordered_map<std::string, std::string> getHeaders(const char *buf) {
    std::unordered_map<std::string, std::string> map;
    std::istringstream stream(buf);
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

void printHeaders(std::unordered_map<std::string, std::string> map){
    for(auto it = map.begin();it != map.end(); it++){
        std::cout << "$> " << it->first << ": " << it->second << std::endl;
    }
}
