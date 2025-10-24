cmake_minimum_required(VERSION 3.15)
project(MiBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir Crow
add_subdirectory(Crow)

# Tu ejecutable
add_executable(servidor src/main.cpp)

# Enlazar con Crow
target_link_libraries(servidor PRIVATE Crow::Crow)

# Incluir tus propios headers si los necesitas
target_include_directories(servidor PRIVATE include)

# Si necesitas threads para WebSocket
find_package(Threads REQUIRED)
target_link_libraries(servidor PRIVATE Threads::Threads)