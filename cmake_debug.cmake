cmake_minimum_required(VERSION 3.10)

message(STATUS "Starting CMake Debug Script")

# Set the correct project directory
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(PROJECT_BINARY_DIR "${CMAKE_CURRENT_LIST_DIR}/build")

# Debug function to print all variables
function(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endfunction()

# Read and process the main CMakeLists.txt
file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" CMAKELISTS_CONTENT)
message(STATUS "CMakeLists.txt content:")
message(STATUS "${CMAKELISTS_CONTENT}")

# Print debug information
message(STATUS "Project Directories:")
message(STATUS "  PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "  PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message(STATUS "CMake Information:")
message(STATUS "  CMAKE_VERSION: ${CMAKE_VERSION}")
message(STATUS "  CMAKE_COMMAND: ${CMAKE_COMMAND}")
message(STATUS "Build Configuration:")
message(STATUS "  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "  CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "  CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

print_all_variables()
