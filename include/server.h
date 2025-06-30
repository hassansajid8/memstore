#pragma once
#include <string>
#include <store.h>

class ServerOptions {
    public:
        int port;
        std::string allowed_origins;
        bool authenticate;

        void load_options();
};

void server(Store store, ServerOptions options);
