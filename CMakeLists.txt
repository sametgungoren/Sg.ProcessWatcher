cmake_minimum_required(VERSION 3.10)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /GL")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s")
endif()
