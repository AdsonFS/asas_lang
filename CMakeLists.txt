# Requer uma versão moderna do CMake
cmake_minimum_required(VERSION 3.16)
project(asas LANGUAGES CXX)

# Configura o padrão C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opções de build
option(ENABLE_TRACE "Enable execution tracing" OFF)
option(ENABLE_WARNINGS "Enable extra compiler warnings" ON)

# Defina tipos de build padrão (Debug / Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Trace enabled: ${ENABLE_TRACE}")

# Adiciona flags específicas de compilação por tipo de build
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Coleta os arquivos .cpp
file(GLOB LIB_SOURCES "src/*.cpp")

# Cria a biblioteca
add_library(asas_lib ${LIB_SOURCES})
target_include_directories(asas_lib PUBLIC include)

# Warnings opcionais
if(ENABLE_WARNINGS)
    target_compile_options(asas_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Define DEBUG_TRACE_EXECUTION se habilitado
if(ENABLE_TRACE)
    target_compile_definitions(asas_lib PUBLIC DEBUG_TRACE_EXECUTION)
endif()

# Cria o executável principal
add_executable(asas main.cpp)
target_link_libraries(asas PRIVATE asas_lib)
target_include_directories(asas PRIVATE include)

if(ENABLE_WARNINGS)
    target_compile_options(asas PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_TRACE)
    target_compile_definitions(asas PRIVATE DEBUG_TRACE_EXECUTION)
endif()

# Habilita testes
enable_testing()
add_subdirectory(tests)

