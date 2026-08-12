#include "AntiSandbox.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>

// --- RESOURCE CHECK / VERIFICAÇÃO DE RECURSOS ---

// Checks if the system has low physical RAM resources (< 4GB), typical of lightweight sandboxes
// Verifica se o sistema possui poucos recursos de RAM física (< 4GB), típico de sandboxes leves
bool AntiSandbox::IsLowResources() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) return false;

    float ramGB = static_cast<float>(statex.ullTotalPhys) / (1024.0f * 1024.0f * 1024.0f);
    return ramGB < 4.0f; 
}

// --- CPU CORE CHECK / VERIFICAÇÃO DE NÚCLEOS DE CPU ---

// Checks if the system has fewer than 2 processor cores, often indicative of analysis environments
// Verifica se o sistema possui menos de 2 núcleos de processador, o que frequentemente indica ambientes de análise
bool AntiSandbox::IsSingleCore() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors < 2;
}

// --- VIRTUAL MACHINE DETECTION / DETECÇÃO DE MÁQUINA VIRTUAL ---

// Checks for known virtual machine driver artifacts on the file system
// Verifica artefatos conhecidos de drivers de máquina virtual no sistema de arquivos
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

// --- EVASION DELAY / ATRASO DE EVASÃO ---

// Executes a heavy mathematical delay loop to waste time and evade time-accelerated sandboxes
// Executa um loop de atraso matemático pesado para desperdiçar tempo e evadir sandboxes aceleradas por tempo
void AntiSandbox::ExecuteHeavyDelay(long long iterations) {
    double dummy = 0.0;
    
    for (long long i = 0; i < iterations; ++i) {
        dummy += std::sin(i) * std::cos(i);
        dummy = std::sqrt(std::abs(dummy + 1.0));
    }

    if (dummy == 0.0001) { 
        std::cout << "[-AntiSandBox-] An impossible condition occurred / Uma condição impossível ocorreu"; 
    }
}