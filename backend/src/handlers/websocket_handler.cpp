#include "handlers/websocket_handler.h"
#include "syscalls/screen_live.h"
#include "syscalls/resources_pc.h"
#include "crow/json.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace Handlers {

WebSocketHandler::WebSocketHandler() : running_(false) {}

WebSocketHandler::~WebSocketHandler() {
    stop();
}

void WebSocketHandler::addConnection(crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.insert(&conn);
    std::cout << " Nueva conexión WebSocket. Total: " << connections_.size() << std::endl;
}

void WebSocketHandler::removeConnection(crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(&conn);
    std::cout << " Conexión WebSocket cerrada. Total: " << connections_.size() << std::endl;
}

void WebSocketHandler::handleMessage(crow::websocket::connection& conn, 
                                     const std::string& message) {
    std::cout << " Mensaje recibido: " << message << std::endl;
    
    // Aquí podrías parsear comandos del cliente si es necesario
    auto json_msg = crow::json::load(message);
    
    if (json_msg && json_msg.has("command")) {
        std::string command = json_msg["command"].s();
        
        if (command == "start_stream") {
            std::cout << "  Iniciando streaming..." << std::endl;
        } else if (command == "stop_stream") {
            std::cout << "  Pausando streaming..." << std::endl;
        }
    }
}

void WebSocketHandler::screenshotLoop() {
    std::cout << " Thread de screenshots iniciado" << std::endl;
    
    while (running_) {
        // Obtener screenshot en Base64
        std::string base64_image = Syscalls::getScreenshotBase64();
        
        if (!base64_image.empty()) {
            // Crear mensaje JSON
            crow::json::wvalue json_msg;
            json_msg["type"] = "screenshot";
            json_msg["data"] = base64_image;
            json_msg["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
            
            std::string message = json_msg.dump();
            
            // Enviar a todos los clientes
            broadcast(message);
        }
        
        // Esperar según FPS configurado (1 FPS = 1000ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / Config::FPS));
    }
    
    std::cout << " Thread de screenshots detenido" << std::endl;
}

void WebSocketHandler::resourcesLoop() {
    std::cout << " Thread de recursos iniciado" << std::endl;
    
    while (running_) {
        // Obtener recursos en JSON
        std::string resources_json = Syscalls::getResourcesJSON();
        
        if (!resources_json.empty()) {
            broadcast(resources_json);
        }
        
        // Actualizar cada 2 segundos
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    std::cout << " Thread de recursos detenido" << std::endl;
}

void WebSocketHandler::start() {
    if (running_) {
        std::cout << "  Handler ya está corriendo" << std::endl;
        return;
    }
    
    running_ = true;
    
    // Iniciar threads
    screenshot_thread_ = std::thread(&WebSocketHandler::screenshotLoop, this);
    resources_thread_ = std::thread(&WebSocketHandler::resourcesLoop, this);
    
    std::cout << " WebSocket Handler iniciado" << std::endl;
}

void WebSocketHandler::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Esperar a que los threads terminen
    if (screenshot_thread_.joinable()) {
        screenshot_thread_.join();
    }
    if (resources_thread_.joinable()) {
        resources_thread_.join();
    }
    
    std::cout << " WebSocket Handler detenido" << std::endl;
}

void WebSocketHandler::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    for (auto* conn : connections_) {
        try {
            conn->send_text(message);
        } catch (const std::exception& e) {
            std::cerr << " Error al enviar mensaje: " << e.what() << std::endl;
        }
    }
}

} // namespace Handlers