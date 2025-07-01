#include <crypto.h>
#include <fstream>
#include <cstring>
#include <stdexcept>

TEA::TEA(){
    // load key from .env
    std::ifstream env(".env");

    bool key_found = false;

    if(!env.is_open()) throw std::runtime_error("$> .env file not found.");

    std::string name, value;

    while(env >> name >> value){
        if(name == "KEY") {
            if(value.size() != 16) throw std::invalid_argument("$> Key must be exactly 16 bytes (16 characters)");
            std::memcpy(key, value.data(), 16);
            key_found = true;
        }
    }

    if(!key_found) throw std::runtime_error("$> KEY not found in .env file");
}

void TEA::encryptBlock(uint32_t& v0, uint32_t& v1){
    uint32_t sum = 0;

    for(int i =0;i<32;i++){
        sum += delta;
        v0 += ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
        v1 += ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
    }
}

void TEA::decryptBlock(uint32_t& v0, uint32_t& v1) {
    uint32_t sum = delta * 32;

    for (int i = 0; i < 32; ++i) {
        v1 -= ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
        v0 -= ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
        sum -= delta;
    }
}

std::string TEA::encrypt(const std::string& plaintext){
    std::string padded = plaintext;

    while(padded.size() % 8 != 0) padded += '\0';

    std::string result;
    for(size_t i=0; i<padded.size(); i+=8){
        uint32_t v0 = *reinterpret_cast<const uint32_t*>(&padded[i]);
        uint32_t v1 = *reinterpret_cast<const uint32_t*>(&padded[i+4]);
        encryptBlock(v0, v1);
        result.append(reinterpret_cast<char*>(&v0), 4);
        result.append(reinterpret_cast<char*>(&v1), 4);
    }

    return result;
}

std::string TEA::decrypt(const std::string& ciphertext) {
    if (ciphertext.size() % 8 != 0) throw std::invalid_argument("Invalid ciphertext size");

    std::string result;
    for (size_t i = 0; i < ciphertext.size(); i += 8) {
        uint32_t v0 = *reinterpret_cast<const uint32_t*>(&ciphertext[i]);
        uint32_t v1 = *reinterpret_cast<const uint32_t*>(&ciphertext[i + 4]);
        decryptBlock(v0, v1);
        result.append(reinterpret_cast<char*>(&v0), 4);
        result.append(reinterpret_cast<char*>(&v1), 4);
    }

    // Remove trailing nulls
    while (!result.empty() && result.back() == '\0') result.pop_back();
    return result;
}