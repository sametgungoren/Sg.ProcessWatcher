# Sg.ProcessWatcher

## Overview
Sg.ProcessWatcher is a lightweight Windows system tray application that monitors and manages processes on your system. It provides a graphical interface for managing blocked processes and runs silently in the system tray.

## Features
- System tray integration with custom icon
- Graphical user interface for managing blocked processes
- Real-time process monitoring and termination
- Persistent storage of blocked process list
- Single instance enforcement
- Automatic process termination for blocked processes
- Logging functionality for monitoring and debugging
- Low CPU and memory footprint
- Windows native look and feel

## User Interface
- System tray icon for quick access
- List view showing blocked processes
- Add/Remove buttons for process management
- Right-click tray menu for quick exit

## Technical Details
- Written in C++17
- Uses Win32 API for native Windows integration
- CMake-based build system
- Multithreaded design for responsive UI
- Resource-efficient implementation
- Includes custom application icon

## File Structure
- `blocked_processes.txt`: Stores the list of processes to block
- `process_watcher.log`: Contains application logs with timestamps
- `sg.ico`: Application icon file

## Build Requirements
- CMake 3.10 or higher
- C++17 compatible compiler (MSVC recommended)
- Windows SDK
- Visual Studio (recommended) or other compatible IDE

## Building the Project
```bash
# Create build directory
mkdir build
cd build

# Generate build files
cmake ..

# Build the project
cmake --build . --config Release
```

## Usage
1. Launch the application
2. The app will appear in the system tray with a custom icon
3. Click the tray icon to show/hide the main window
4. Use the main window to:
   - View currently blocked processes
   - Add new processes to block
   - Remove processes from the block list
5. Right-click the tray icon to access the context menu

## Notes
- The application automatically ensures only one instance is running
- Requires appropriate permissions to terminate processes
- Blocked processes are stored persistently between sessions
- Logs are written to `process_watcher.log` for troubleshooting

## Implementation Details
- Uses Windows API for process monitoring and termination
- Implements a separate monitoring thread for process checking
- Utilizes Windows common controls for the user interface
- Includes system tray integration with custom icon support
- Implements proper cleanup and resource management

## Security Considerations
- Requires appropriate Windows permissions to terminate processes
- Exercise caution when adding system-critical processes to the block list
- Logging functionality helps track any unauthorized termination attempts
