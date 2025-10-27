
// URL del WebSocket - CAMBIAR según tu configuración
const WS_URL = 'ws://10.150.1.233:8080/ws';

class WebSocketService {
  constructor() {
    this.ws = null;
    this.listeners = {
      screenshot: [],
      resources: [],
      open: [],
      close: [],
      error: [],
    };
  }

  // Conectar al WebSocket
  connect() {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      console.log('WebSocket ya está conectado');
      return;
    }

    console.log('Conectando a WebSocket:', WS_URL);
    this.ws = new WebSocket(WS_URL);

    // Evento: conexión establecida
    this.ws.onopen = () => {
      console.log('WebSocket conectado');
      this.notifyListeners('open', { connected: true });
    };

    // Evento: mensaje recibido
    this.ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        console.log('Mensaje recibido:', data.type);
        
        // Dependiendo del tipo de mensaje, notificamos a los listeners correspondientes
        if (data.type === 'screenshot') {
          this.notifyListeners('screenshot', data);
        } else if (data.type === 'resources') {
          this.notifyListeners('resources', data);
        }
      } catch (error) {
        console.error('Error al parsear mensaje:', error);
      }
    };

    // Evento: conexión cerrada
    this.ws.onclose = () => {
      console.log(' WebSocket desconectado');
      this.notifyListeners('close', { connected: false });
    };

    // Evento: error de conexión
    this.ws.onerror = (error) => {
      console.error(' Error en WebSocket:', error);
      this.notifyListeners('error', error);
    };
  }

  // Desconectar WebSocket
  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
      console.log('WebSocket desconectado manualmente');
    }
  }

  // Enviar mensaje al servidor (por si quieres enviar comandos)
  send(message) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    } else {
      console.warn('WebSocket no está conectado');
    }
  }

  // Registrar un listener para un evento específico
  on(event, callback) {
    if (this.listeners[event]) {
      this.listeners[event].push(callback);
    }
  }

  // Remover un listener
  off(event, callback) {
    if (this.listeners[event]) {
      this.listeners[event] = this.listeners[event].filter(cb => cb !== callback);
    }
  }

  // Notificar a todos los listeners de un evento
  notifyListeners(event, data) {
    if (this.listeners[event]) {
      this.listeners[event].forEach(callback => callback(data));
    }
  }
}

// Exportamos una instancia única (singleton)
const websocketService = new WebSocketService();
export default websocketService;