cmake_minimum_required(VERSION 3.15)
project(USACRemoteDesktop 
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==================== Dependencias ====================
add_subdirectory(Crow)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# ==================== Sources ====================
set(SYSCALLS_SOURCES
    src/syscalls/screen_live.cpp
    src/syscalls/mouse_tracking.cpp
    src/syscalls/mouse_action.cpp
    src/syscalls/keyboard_caption.cpp
    src/syscalls/resources_pc.cpp
)

set(HANDLERS_SOURCES
    src/handlers/websocket_handler.cpp
    src/handlers/http_handler.cpp
)

set(UTILS_SOURCES
    src/utils/base64.cpp
)

# ==================== Ejecutable ====================
add_executable(servidor 
    src/main.cpp
    ${SYSCALLS_SOURCES}
    ${HANDLERS_SOURCES}
    ${UTILS_SOURCES}
)

# ==================== Headers ====================
target_include_directories(servidor 
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
)

# ==================== Linkeo ====================
target_link_libraries(servidor 
    PRIVATE 
        Crow::Crow
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

# ==================== Flags de compilación ====================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(servidor PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
    )
endif()

# Mensaje de configuración
message(STATUS "===========================================")
message(STATUS "USAC Linux Remote Desktop - Backend")
message(STATUS "Versión: ${PROJECT_VERSION}")
message(STATUS "Compilador: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "===========================================")