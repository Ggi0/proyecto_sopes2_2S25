import { useState, useEffect, useCallback } from 'react';
import websocketService from '../services/websocketService';

export const useWebSocket = () => {
  // Estado para saber si está conectado
  const [isConnected, setIsConnected] = useState(false);
  
  // Estado para la última imagen recibida
  const [screenshot, setScreenshot] = useState(null);
  
  // Estado para los recursos del sistema
  const [resources, setResources] = useState({
    cpu: 0,
    ram: 0,
    ram_total: 0,
    ram_used: 0,
    ram_free: 0,
  });

  // Función para conectar al WebSocket
  const connect = useCallback(() => {
    websocketService.connect();
  }, []);

  // Función para desconectar
  const disconnect = useCallback(() => {
    websocketService.disconnect();
  }, []);

  useEffect(() => {
    // Listeners para los diferentes eventos

    // Cuando se conecta
    const handleOpen = () => {
      console.log('useWebSocket: Conectado');
      setIsConnected(true);
    };

    // Cuando se desconecta
    const handleClose = () => {
      console.log('useWebSocket: Desconectado');
      setIsConnected(false);
    };

    // Cuando llega un screenshot
    const handleScreenshot = (data) => {
      // data.data contiene el base64 de la imagen
      setScreenshot({
        image: data.data,
        timestamp: data.timestamp,
      });
    };

    // Cuando llegan recursos del sistema
    const handleResources = (data) => {
      setResources({
        cpu: data.cpu_usage || 0,
        ram: data.ram_usage || 0,
        ram_total: data.ram_total || 0,
        ram_used: data.ram_used || 0,
        ram_free: data.ram_free || 0,
      });
    };

    // Registramos los listeners
    websocketService.on('open', handleOpen);
    websocketService.on('close', handleClose);
    websocketService.on('screenshot', handleScreenshot);
    websocketService.on('resources', handleResources);

    // Cleanup: removemos los listeners cuando se desmonta el componente
    return () => {
      websocketService.off('open', handleOpen);
      websocketService.off('close', handleClose);
      websocketService.off('screenshot', handleScreenshot);
      websocketService.off('resources', handleResources);
    };
  }, []);

  return {
    isConnected,
    screenshot,
    resources,
    connect,
    disconnect,
  };
};