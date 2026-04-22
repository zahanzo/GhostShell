#include "DownloadManager.hpp"
#include <iostream>

DownloadManager::DownloadManager() : hInternet(nullptr) {
    hInternet = InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36", 
                              INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
}

DownloadManager::~DownloadManager() {
    if (hInternet) {
        InternetCloseHandle(hInternet);
    }
}

std::string DownloadManager::FetchRemoteCommand(const std::string& url) {
    if (!hInternet) return "";

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hConnect) return "";

    std::string response;
    char buffer[4096];
    DWORD bytesRead = 0;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }

    InternetCloseHandle(hConnect);
    return response;
}