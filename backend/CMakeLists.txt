cmake_minimum_required(VERSION 3.15)
project(MiBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir Crow
add_subdirectory(Crow)

# Flags de compilación
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Incluir directorios
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/Crow/include)

# Encontrar OpenSSL (para Base64)
find_package(OpenSSL REQUIRED)

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/syscalls/screen_live.cpp
    src/syscalls/mouse_tracking.cpp
    src/syscalls/mouse_action.cpp
    src/syscalls/keyboard_caption.cpp
    src/syscalls/resources_pc.cpp
    src/handlers/websocket_handler.cpp
    src/handlers/http_handler.cpp
    src/utils/base64.cpp
)

# Tu ejecutable
add_executable(servidor ${SOURCE})

# Enlazar con Crow
target_link_libraries(servidor PRIVATE Crow::Crow)

# Incluir tus propios headers si los necesitas
target_include_directories(servidor PRIVATE include)

# Si necesitas threads para WebSocket
find_package(Threads REQUIRED)
target_link_libraries(servidor PRIVATE Threads::Threads)


# Enlazar librerías
target_link_libraries(servidor
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
)