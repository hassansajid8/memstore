#pragma once
#include <string>
#include <store.h>

class ServerOptions {
    public:
        int port;
        std::string allowed_origins;
        std::string auth_key;
        bool authorize;
        bool encrypt;

        void load_options();
};

void server(Store store, ServerOptions options);
