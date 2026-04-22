#ifndef OBFUSCATOR_HPP
#define OBFUSCATOR_HPP

#include <string>
#include <array>

template <size_t N>
class ObfuscatedString {
public:
    static constexpr char key = 0x7A; 
    std::array<char, N> encrypted_data;

    constexpr ObfuscatedString(const char* str) : encrypted_data{} {
        for (size_t i = 0; i < N; ++i) {
            encrypted_data[i] = str[i] ^ key;
        }
    }

    std::string decrypt() const {
        std::string decrypted;
        decrypted.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            decrypted += (encrypted_data[i] ^ key);
        }
        return decrypted;
    }
};

#define PROTECT(str) []() { \
    constexpr auto obfs = ObfuscatedString<sizeof(str)>(str); \
    return obfs.decrypt(); \
}()

#endif