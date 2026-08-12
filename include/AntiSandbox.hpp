#ifndef ANTISANDBOX_HPP
#define ANTISANDBOX_HPP

#include <windows.h>
#include <iostream>
#include <filesystem>
#include <cmath>
#include <complex>

class AntiSandbox {
public:
    static bool IsLowResources();
    static bool IsSingleCore();
    static bool IsVirtualMachine();
    static void ExecuteHeavyDelay(long long iterations);

};

#endif // ANTISANDBOX_HPP