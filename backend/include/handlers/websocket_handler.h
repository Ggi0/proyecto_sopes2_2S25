#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include "crow/websocket.h"
#include <memory>
#include <set>
#include <mutex>
#include <thread>
#include <atomic>

namespace Handlers {

/**
 * @brief Gestor de conexiones WebSocket para streaming en tiempo real
 * 
 * Maneja múltiples conexiones simultáneas y envía screenshots
 * y recursos del sistema a todos los clientes conectados
 */
class WebSocketHandler {
private:
    std::set<crow::websocket::connection*> connections_;  // Conexiones activas
    std::mutex connections_mutex_;                        // Mutex para thread-safety
    std::thread screenshot_thread_;                       // Thread para screenshots
    std::thread resources_thread_;                        // Thread para recursos
    std::atomic<bool> running_;                           // Flag de ejecución

    /**
     * @brief Loop que envía screenshots continuamente
     */
    void screenshotLoop();

    /**
     * @brief Loop que envía recursos del sistema continuamente
     */
    void resourcesLoop();

public:
    WebSocketHandler();
    ~WebSocketHandler();

    /**
     * @brief Registra una nueva conexión
     */
    void addConnection(crow::websocket::connection& conn);

    /**
     * @brief Elimina una conexión cerrada
     */
    void removeConnection(crow::websocket::connection& conn);

    /**
     * @brief Maneja mensajes entrantes del cliente
     */
    void handleMessage(crow::websocket::connection& conn, 
                      const std::string& message);

    /**
     * @brief Inicia los threads de transmisión
     */
    void start();

    /**
     * @brief Detiene los threads de transmisión
     */
    void stop();

    /**
     * @brief Envía un mensaje a todos los clientes conectados
     */
    void broadcast(const std::string& message);
};

} // namespace Handlers

#endif // WEBSOCKET_HANDLER_H