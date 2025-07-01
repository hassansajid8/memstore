#pragma once

#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <cstdint>

// Tiny Encryption Module
class TEA{
    private:
        uint32_t delta = 0x9e3779b9;
        uint32_t key[4];

    public:
        TEA();

        void encryptBlock(uint32_t&, uint32_t&);
        void decryptBlock(uint32_t&, uint32_t&);

        std::string encrypt(const std::string& plaintext);
        std::string decrypt(const std::string& ciphertext);

};

#endif