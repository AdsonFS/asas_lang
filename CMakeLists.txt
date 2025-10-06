cmake_minimum_required(VERSION 4.0)
project(asas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option para habilitar o trace
option(ENABLE_TRACE "Enable execution tracing" OFF)

# Collect all cpp files in src directory
file(GLOB LIB_SOURCES "src/*.cpp")

# Create a library target
add_library(asas_lib ${LIB_SOURCES})
target_include_directories(asas_lib PUBLIC include)

# Define DEBUG_TRACE_EXECUTION if ENABLE_TRACE is ON
if(ENABLE_TRACE)
    target_compile_definitions(asas_lib PUBLIC DEBUG_TRACE_EXECUTION)
endif()

# Create an executable target
add_executable(asas main.cpp)
target_link_libraries(asas PRIVATE asas_lib)
target_include_directories(asas PRIVATE include)

# Define DEBUG_TRACE_EXECUTION for the executable if ENABLE_TRACE is ON
if(ENABLE_TRACE)
    target_compile_definitions(asas PRIVATE DEBUG_TRACE_EXECUTION)
endif()

# Enable testing
enable_testing()
add_subdirectory(tests)
