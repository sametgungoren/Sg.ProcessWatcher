# ProcessWatch

## Overview
ProcessWatch is a lightweight, low-resource Windows application that monitors and terminates specified processes.

## Features
- Read blocked process names from `blocked_processes.txt`
- Continuously monitor running processes
- Automatically terminate processes listed in the blocked list
- Low CPU and memory usage
- Easy to configure

## Usage

### Adding Blocked Processes
Create a `blocked_processes.txt` file in the same directory as the executable. 
Each line should contain a process name to block (e.g., `chrome.exe`, `notepad.exe`).

### Running the Application
1. Launch the executable
2. The application will start monitoring processes immediately
3. Processes listed in `blocked_processes.txt` will be terminated if they start

### Command-Line Usage
You can also add blocked processes directly via command line:
```
ProcessWatch.exe chrome.exe notepad.exe
```

## Build Requirements
- CMake 3.10 or later
- C++17 compatible compiler (MSVC or GCC)
- Windows SDK

## Compilation
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Notes
- Requires Windows administrative privileges to terminate processes
- Use with caution to avoid unintended system disruption
