#include "crow/app.h"
#include "crow/websocket.h"
#include "crow/json.h"
#include "crow/http_request.h"
#include "crow/http_response.h"

#include <unistd.h>     // para llamadas al sistema
#include <sys/types.h>  // para tipos como pid_t

int main() {
    crow::SimpleApp app;

    // Ruta HTTP
    CROW_ROUTE(app, "/")([](){
        return "Servidor Crow activo";
    });

    // Ruta WebSocket
    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([](crow::websocket::connection& conn){
        std::cout << "WebSocket abierto\n";
    })
    .onmessage([](crow::websocket::connection& conn, const std::string& msg, bool){
        conn.send_text("Echo: " + msg);
    });

    // Ruta que usa llamada al sistema
    CROW_ROUTE(app, "/pid")([](){
        pid_t pid = getpid();  // llamada al sistema
        return "PID del proceso: " + std::to_string(pid);
    });

    app.port(8080).multithreaded().run();
}
