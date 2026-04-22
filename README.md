# 🛡️ GhostShell: Advanced Evasion & Persistence Toolkit

https://github.com/user-attachments/assets/28340dde-fa4d-4a1a-b1a2-1a32ce38d0a3


This repository contains a Proof of Concept (PoC) for a highly stealthy, fileless Reverse Shell developed in **C++20**. Designed as an offensive security research tool, this project demonstrates advanced interaction with the Windows API (Win32 and COM), focusing on in-memory execution, sandbox evasion, and privilege-escalated persistence.

> ⚠️ **Security and Ethics Warning:** This project was created strictly for **educational and Information Security research purposes**. Using this tool on systems without prior consent is illegal. 

---

## ⚙️ Execution Flow (The Kill Chain)

The malware follows a strict, step-by-step execution pipeline designed to bypass static heuristics and dynamic analysis (Sandboxes).

### Step 1: Interface Obfuscation (UI Detachment)
Immediately upon execution, the binary hides its own execution context. It queries the active window using `GetConsoleWindow()` and forces a `ShowWindow(hWnd, SW_HIDE)` call. This prevents the console subsystem from flashing on the target's screen, ensuring a silent initialization.

### Step 2: Environmental Fingerprinting (Anti-Sandbox)
Before establishing any network connections, the malware inspects its host environment to detect virtualization or automated analysis tools:
* **Hardware Checks:** Uses `GlobalMemoryStatusEx()` and `GetSystemInfo()` to verify if the system has at least 4GB of RAM and multiple CPU cores.
* **Driver Enumeration:** Checks the filesystem for known hypervisor artifacts (e.g., `VBoxMouse.sys`, `vmhgfs.sys`). 
If any check fails, the process terminates silently (`return 0`).

### Step 3: Temporal Evasion (Heavy Delay)
To bypass dynamic analysis sandboxes that have strict execution time limits (usually 2-3 minutes), the malware executes a deterministic, CPU-intensive mathematical loop. This exhausts the sandbox's monitoring timer before the actual malicious payload is unpacked.

### Step 4: COM-Based Persistence
Instead of modifying the highly monitored `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` registry key, the malware communicates directly with the **Task Scheduler 2.0 COM API**:
* Initializes COM via `CoInitializeEx`.
* Requests the `ITaskService` and `IPrincipal` interfaces.
* Creates a hidden scheduled task triggered by `TASK_TRIGGER_LOGON`.
* Elevates the execution context by requesting `TASK_RUNLEVEL_HIGHEST`, completely bypassing standard startup monitors and ensuring the shell survives reboots with maximum privileges.

### Step 5: Fileless C2 Retrieval & Decryption
The command-and-control (C2) URL is protected against static string extraction (like `strings.exe` or YARA rules) using a custom `constexpr` template that encrypts the URL via XOR at **compile-time**.
At runtime, the URL is decrypted in memory. The `WinInet` library (`InternetOpenUrlA`, `InternetReadFile`) is then used to fetch the target IP and Port dynamically, storing them strictly in the RAM (Fileless), without touching the disk.

### Step 6: Handle Hijacking & Native Reverse Shell
The final stage establishes the connection and grants remote control:
* Initializes the networking stack via `WSAStartup`.
* Creates an overlapped TCP socket using `WSASocketA`.
* Converts the string IP to a network-byte-order binary structure using the modern `inet_pton` API.
* Executes a new `cmd.exe` instance via `CreateProcessA`. Crucially, it manipulates the `STARTUPINFO` structure by setting the `STARTF_USESTDHANDLES` flag. This binds the Standard Input (`STDIN`), Output (`STDOUT`), and Error (`STDERR`) handles directly to the open Winsock handle. 
* The result is a fully interactive remote shell without relying on external binaries like `nc.exe`.

---

### 📡 Remote C2 Configuration (Pastebin Example)

The `DownloadManager` module is designed to fetch the target IP and Port dynamically from a remote server. The parser in `main.cpp` strictly splits the downloaded string using the pipe (`|`) character.

**Required File Structure:**
The remote text file must contain exactly the IP address and the port, separated by a single pipe. It must contain **no** extra spaces, protocols (like `tcp://`), or trailing newlines.

**Format:**
```text
<IP_ADDRESS>|<PORT>
```

**Pastebin Deployment Example:**
1. Create a new paste containing only your listener's IP and Port (e.g., your Ngrok IP or local Kali IP):
```text
3.14.15.92|12345
```
2. Save the paste and copy the **Raw** link (e.g., `https://pastebin.com/raw/XYZ123`).
3. Update the `PROTECT("...")` macro in `main.cpp` with this **Raw URL** before compiling. 

*Note: If the URL provided is not the Raw version, the `WinInet` HTTP response will download the entire HTML page structure, which will cause the `std::stoi` parser to crash silently during execution.*

---

## 💻 Compilation & Deployment

To maintain its stealth profile and avoid dropping dependencies (like `vcruntime` or `libstdc++`), compile the binary statically using MinGW-w64 (GCC) with the C++20 standard enabled:

```bash
g++ main.cpp AntiSandbox.cpp DownloadManager.cpp PersistenceManager.cpp -o win_service.exe -std=c++20 -static -static-libgcc -static-libstdc++ -mwindows -s -lws2_32 -lwininet -lole32 -loleaut32 -luuid
```

**Compiler Flags Breakdown:**
* `-std=c++20`: Required for the compile-time XOR string obfuscation (`constexpr`).
* `-static`: Embeds all standard C++ libraries directly into the binary.
* `-mwindows`: Instructs the Windows subsystem to treat the executable as a GUI application, natively preventing the CMD console window from spawning.
* `-s`: Strips all debugging symbols, significantly reducing the binary size and hindering reverse engineering efforts.
