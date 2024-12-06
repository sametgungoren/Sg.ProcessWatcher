cmake_minimum_required(VERSION 3.10)

# Enable CMake debugging
set(CMAKE_DEBUG_TARGET_PROPERTIES
    INCLUDE_DIRECTORIES
    COMPILE_DEFINITIONS
    COMPILE_OPTIONS
    COMPILE_FEATURES
)

project(Sg.ProcessWatcher)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add icon resource
set(RESOURCE_FILES icon.rc)

# Windows GUI uygulaması için gerekli ayarlar
if(WIN32)
    add_executable(Sg.ProcessWatcher WIN32 main.cpp ${RESOURCE_FILES})
    target_link_libraries(Sg.ProcessWatcher 
        kernel32 
        user32 
        shell32
        comctl32
    )
else()
    add_executable(Sg.ProcessWatcher main.cpp ${RESOURCE_FILES})
endif()

# Subsystem ayarı
set_target_properties(Sg.ProcessWatcher PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

# Set compiler flags for optimization and low resource usage
if(MSVC)
    # Debug configuration
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /RTC1")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
    
    # Release configuration
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s")
endif()

# Enable Visual Studio debugging
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sg.ProcessWatcher)
