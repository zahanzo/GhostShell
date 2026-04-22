#ifndef DOWNLOADMANAGER_HPP
#define DOWNLOADMANAGER_HPP

#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>


#pragma comment(lib, "wininet.lib")

class DownloadManager {
private:
    HINTERNET hInternet;

public:
    DownloadManager();
    ~DownloadManager();

    std::string FetchRemoteCommand(const std::string& url);
};

#endif