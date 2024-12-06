#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <commctrl.h>
#include <tlhelp32.h>
#include <psapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM 3000
#define ID_LIST_BLOCKED_PROCESSES 5001
#define ID_BTN_ADD_PROCESS 5002
#define ID_BTN_REMOVE_PROCESS 5003
#define IDI_MAINICON 101

// Global mutex to ensure single instance
HANDLE g_hMutex = NULL;

// Function to find and close previous instances
void ClosePreviousInstances() {
    DWORD currentProcessId = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID != currentProcessId) {
                std::wstring wProcessName = pe32.szExeFile;
                std::string processName(wProcessName.begin(), wProcessName.end());
                
                if (_stricmp(processName.c_str(), "Sg.ProcessWatcher.exe") == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess != NULL) {
                        TerminateProcess(hProcess, 1);
                        CloseHandle(hProcess);
                    }
                }
            }
        } while (Process32NextW(snapshot, &pe32));
    }

    CloseHandle(snapshot);
}

class ProcessWatcher {
private:
    HWND hMainWnd;
    HWND hListBox;
    HWND hAddButton;
    HWND hRemoveButton;
    NOTIFYICONDATAW nid;
    std::vector<std::string> blockedProcesses;
    std::atomic<bool> isRunning{true};
    std::thread monitorThread;
    std::atomic<bool> shouldExit{false};  // Changed from true to false
    HMENU hTrayMenu;  

    void initSystemTray() {
        nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = hMainWnd;
        nid.uID = ID_TRAY_APP_ICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        
        // Load custom icon
        nid.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_MAINICON));
        
        wcscpy_s(nid.szTip, L"Sg Process Watcher");

        Shell_NotifyIconW(NIM_ADD, &nid);
    }

    void readBlockedProcesses() {
        std::ifstream file("blocked_processes.txt");
        std::string process;
        blockedProcesses.clear();

        // Dosya yoksa oluştur
        if (!file.good()) {
            std::ofstream createFile("blocked_processes.txt");
            createFile.close();
            std::cout << "Created blocked_processes.txt" << std::endl;
            return;
        }

        while (std::getline(file, process)) {
            if (!process.empty()) {
                blockedProcesses.push_back(process);
                SendMessageW(hListBox, LB_ADDSTRING, 0, 
                    reinterpret_cast<LPARAM>(std::wstring(process.begin(), process.end()).c_str()));
            }
        }
    }

    void killProcess(const std::string& processName) {
        // Detailed logging for process termination attempt
        std::cout << "Attempting to terminate process: " << processName << std::endl;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            std::cerr << "ERROR: Failed to create process snapshot for " << processName 
                      << ". Error code: " << error << std::endl;
            return;
        }

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(pe32);

        bool processFound = false;
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                std::wstring wProcessName = pe32.szExeFile;
                std::string currentProcessName(wProcessName.begin(), wProcessName.end());
                
                // Case-insensitive comparison
                if (_stricmp(currentProcessName.c_str(), processName.c_str()) == 0) {
                    processFound = true;
                    std::cout << "Found process to terminate: " << currentProcessName 
                              << " (PID: " << pe32.th32ProcessID << ")" << std::endl;

                    // Skip system processes
                    if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4) {
                        std::cout << "Skipping system process" << std::endl;
                        continue;
                    }

                    // Try multiple termination strategies
                    bool terminated = false;

                    // Strategy 1: Direct termination with PROCESS_TERMINATE
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess != NULL) {
                        if (TerminateProcess(hProcess, 1)) {
                            terminated = true;
                            std::cout << "Successfully terminated process with PROCESS_TERMINATE" << std::endl;
                        }
                        CloseHandle(hProcess);
                    }

                    // Strategy 2: Full access rights if first attempt failed
                    if (!terminated) {
                        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                        if (hProcess != NULL) {
                            if (TerminateProcess(hProcess, 1)) {
                                terminated = true;
                                std::cout << "Successfully terminated process with PROCESS_ALL_ACCESS" << std::endl;
                            }
                            CloseHandle(hProcess);
                        }
                    }

                    // Strategy 3: Use taskkill command as a last resort
                    if (!terminated) {
                        std::string cmd = "taskkill /F /IM " + processName + " /T";
                        std::cout << "Attempting taskkill command: " << cmd << std::endl;
                        int result = system(cmd.c_str());
                        if (result == 0) {
                            terminated = true;
                            std::cout << "Successfully terminated process with taskkill" << std::endl;
                        }
                    }

                    if (!terminated) {
                        std::cerr << "Failed to terminate process after all attempts" << std::endl;
                    }
                }
            } while (Process32NextW(snapshot, &pe32));
        }

        CloseHandle(snapshot);

        if (!processFound) {
            std::cout << "No matching process found for: " << processName << std::endl;
        }
    }

    void saveBlockedProcesses() {
        std::ofstream file("blocked_processes.txt");
        for (const auto& process : blockedProcesses) {
            file << process << std::endl;
        }
    }

    void addBlockedProcess(const std::string& processName) {
        if (std::find(blockedProcesses.begin(), blockedProcesses.end(), processName) == blockedProcesses.end()) {
            blockedProcesses.push_back(processName);
            SendMessageW(hListBox, LB_ADDSTRING, 0, 
                reinterpret_cast<LPARAM>(std::wstring(processName.begin(), processName.end()).c_str()));
            saveBlockedProcesses();
        }
    }

    void removeBlockedProcess() {
        int selectedIndex = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
        if (selectedIndex != LB_ERR) {
            wchar_t buffer[MAX_PATH];
            SendMessageW(hListBox, LB_GETTEXT, selectedIndex, (LPARAM)buffer);
            std::wstring wProcessName = buffer;
            std::string processName(wProcessName.begin(), wProcessName.end());

            blockedProcesses.erase(
                std::remove(blockedProcesses.begin(), blockedProcesses.end(), processName),
                blockedProcesses.end()
            );

            SendMessage(hListBox, LB_DELETESTRING, selectedIndex, 0);
            saveBlockedProcesses();
        }
    }

    void monitorLoop() {
        std::cout << "Starting process monitoring loop..." << std::endl;
        std::cout << "Blocked Processes: ";
        for (const auto& process : blockedProcesses) {
            std::cout << process << " ";
        }
        std::cout << std::endl;

        while (isRunning) {
            if (shouldExit) {
                std::cout << "Monitoring loop exiting due to shouldExit flag" << std::endl;
                break;  
            }

            // Skip if no processes to monitor
            if (blockedProcesses.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            try {
                HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (snapshot != INVALID_HANDLE_VALUE) {
                    PROCESSENTRY32W pe32;
                    pe32.dwSize = sizeof(pe32);

                    if (Process32FirstW(snapshot, &pe32)) {
                        do {
                            if (!isRunning || shouldExit) break;

                            std::wstring wProcessName = pe32.szExeFile;
                            std::string processName(wProcessName.begin(), wProcessName.end());

                            // Skip system processes
                            if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4) {
                                continue;
                            }

                            for (const auto& blockedProcess : blockedProcesses) {
                                if (_stricmp(processName.c_str(), blockedProcess.c_str()) == 0) {
                                    std::cout << "Found blocked process: " << processName 
                                              << " (PID: " << pe32.th32ProcessID << ")" << std::endl;
                                    
                                    try {
                                        killProcess(blockedProcess);
                                    }
                                    catch (const std::exception& e) {
                                        std::cerr << "Error killing process: " << e.what() << std::endl;
                                    }
                                    break;
                                }
                            }
                        } while (Process32NextW(snapshot, &pe32));
                    }
                    CloseHandle(snapshot);
                } else {
                    std::cerr << "Failed to create process snapshot. Error: " << GetLastError() << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error in monitoring loop: " << e.what() << std::endl;
            }

            // Sleep between iterations to reduce CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << "Process monitoring loop ended." << std::endl;
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        ProcessWatcher* pWatcher = reinterpret_cast<ProcessWatcher*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (message) {
            case WM_CREATE: {
                // Store the ProcessWatcher pointer
                CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, 
                    reinterpret_cast<LONG_PTR>(pCS->lpCreateParams));
                return 0;
            }

            case WM_TRAYICON:
                if (lParam == WM_RBUTTONUP) {
                    // Create context menu
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, L"Exit");
                    
                    // Get cursor position
                    POINT pt;
                    GetCursorPos(&pt);
                    
                    // Show context menu
                    SetForegroundWindow(hWnd);
                    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, 
                                   pt.x, pt.y, 0, hWnd, NULL);
                    DestroyMenu(hMenu);
                }
                else if (lParam == WM_LBUTTONUP) {
                    ShowWindow(hWnd, SW_SHOW);
                    SetForegroundWindow(hWnd);
                }
                break;

            case WM_SYSCOMMAND:
                if (wParam == SC_MINIMIZE) {
                    ShowWindow(hWnd, SW_HIDE);
                    return 0;
                }
                break;

            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case ID_TRAY_EXIT_CONTEXT_MENU_ITEM:
                        if (pWatcher) {
                            pWatcher->shutdown();
                        }
                        break;
                    case ID_BTN_ADD_PROCESS: {
                        if (pWatcher) {
                            wchar_t processName[MAX_PATH];
                            if (GetWindowTextW(GetDlgItem(hWnd, ID_BTN_ADD_PROCESS), processName, MAX_PATH)) {
                                std::wstring wProcessName = processName;
                                std::string processNameStr(wProcessName.begin(), wProcessName.end());
                                pWatcher->addBlockedProcess(processNameStr);
                            }
                        }
                        break;
                    }
                    case ID_BTN_REMOVE_PROCESS:
                        if (pWatcher) {
                            pWatcher->removeBlockedProcess();
                        }
                        break;
                }
                break;

            case WM_CLOSE:
                if (pWatcher) {
                    pWatcher->shutdown();
                }
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    void createMainWindow() {
        // Register window class
        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = L"ProcessWatcherClass";
        
        // Load custom icon
        wc.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_MAINICON));

        RegisterClassExW(&wc);

        // Create main window
        hMainWnd = CreateWindowExW(
            0, L"ProcessWatcherClass", L"Sg Process Watcher",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
            NULL, NULL, GetModuleHandle(NULL), this
        );

        // Create list box
        hListBox = CreateWindowExW(
            0, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
            10, 10, 370, 240,
            hMainWnd, (HMENU)ID_LIST_BLOCKED_PROCESSES, 
            GetModuleHandle(NULL), NULL
        );

        // Create buttons
        hAddButton = CreateWindowExW(
            0, L"BUTTON", L"Add Process",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 260, 100, 25,
            hMainWnd, (HMENU)ID_BTN_ADD_PROCESS,
            GetModuleHandle(NULL), NULL
        );

        hRemoveButton = CreateWindowExW(
            0, L"BUTTON", L"Remove Process",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 260, 100, 25,
            hMainWnd, (HMENU)ID_BTN_REMOVE_PROCESS,
            GetModuleHandle(NULL), NULL
        );

        // Initialize system tray
        initSystemTray();
    }

public:
    ProcessWatcher() {
        createMainWindow();
        readBlockedProcesses();
        
        // Start monitoring thread
        monitorThread = std::thread(&ProcessWatcher::monitorLoop, this);
    }

    ~ProcessWatcher() {
        // Ensure thread is stopped
        isRunning = false;
        shouldExit = true;
        
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
    }

    void shutdown() {
        // Set flags to signal shutdown
        isRunning = false;
        shouldExit = true;

        // Remove system tray icon
        Shell_NotifyIconW(NIM_DELETE, &nid);

        // Post quit message to break the message loop
        PostMessage(hMainWnd, WM_QUIT, 0, 0);

        // Join the monitoring thread if it's running
        if (monitorThread.joinable()) {
            monitorThread.join();
        }

        // Destroy the main window
        DestroyWindow(hMainWnd);
    }

    void run() {
        // Start minimized in system tray
        ShowWindow(hMainWnd, SW_HIDE);
        
        // Message loop
        MSG msg;
        while (GetMessageW(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    // Create a mutex to ensure single instance
    g_hMutex = CreateMutexW(NULL, FALSE, L"ProcessWatcherMutex");
    if (g_hMutex == NULL) {
        MessageBoxW(NULL, L"Failed to create mutex", L"Error", MB_ICONERROR);
        return 1;
    }

    // Check if another instance is running
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        ClosePreviousInstances();
    }

    ProcessWatcher watcher;
    watcher.run();
    return 0;
}
