#ifndef OBFUSCATOR_HPP
#define OBFUSCATOR_HPP

#include <string>
#include <array>

// Classe que realiza o XOR em tempo de compilação
template <size_t N>
class ObfuscatedString {
public:
    // Chave de ofuscação (pode trocar para qualquer valor entre 0x01 e 0xFF)
    static constexpr char key = 0x7A; 
    std::array<char, N> encrypted_data;

    // O construtor constexpr força o compilador a executar o loop no build
    constexpr ObfuscatedString(const char* str) : encrypted_data{} {
        for (size_t i = 0; i < N; ++i) {
            encrypted_data[i] = str[i] ^ key;
        }
    }

    // Retorna a string original apenas na memória RAM
    std::string decrypt() const {
        std::string decrypted;
        decrypted.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            decrypted += (encrypted_data[i] ^ key);
        }
        return decrypted;
    }
};

// O Macro PROTECT cria uma "Lambda" que descriptografa a string na hora do uso
#define PROTECT(str) []() { \
    constexpr auto obfs = ObfuscatedString<sizeof(str)>(str); \
    return obfs.decrypt(); \
}()

#endif