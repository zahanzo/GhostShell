#ifndef PERSISTENCE_MANAGER_HPP
#define PERSISTENCE_MANAGER_HPP

#include <windows.h>
#include <taskschd.h>
#include <comdef.h>
#include <iostream>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")

class PersistenceManager {
public:
    static bool CreateLogonTask(const std::wstring& taskName, const std::wstring& exePath);
};

#endif