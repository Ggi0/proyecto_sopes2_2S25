import { useRef, useEffect, useState } from 'react';
import { useAuth } from '../hooks/useAuth';
import apiService from '../services/apiService';

const RemoteDesktop = ({ screenshot }) => {
  const canvasRef = useRef(null);
  const { token, canControl } = useAuth();
  const [canvasSize, setCanvasSize] = useState({ width: 1280, height: 800 });

  // Dibujamos la imagen recibida en el canvas
  useEffect(() => {
    if (!screenshot || !canvasRef.current) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const img = new Image();

    img.onload = () => {
      // Limpiar canvas
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Dibujar imagen
      ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
    };

    // El backend envía "data:image/jpeg;base64,..." directamente o solo el base64?
    // Según tu código, envía solo el base64, así que agregamos el prefijo
    img.src = `data:image/jpeg;base64,${screenshot.image}`;
  }, [screenshot]);

  // Handler para clicks en el canvas
  const handleCanvasClick = async (event) => {
    if (!canControl()) {
      console.log('No tienes permisos de control');
      return;
    }

    const canvas = canvasRef.current;
    const rect = canvas.getBoundingClientRect();
    
    // Calcular coordenadas relativas al canvas
    const canvasX = event.clientX - rect.left;
    const canvasY = event.clientY - rect.top;
    
    // Escalar a las coordenadas reales del escritorio remoto (1280x800)
    const realX = Math.floor((canvasX / rect.width) * 1280);
    const realY = Math.floor((canvasY / rect.height) * 800);
    
    // Determinar qué botón se presionó (1=izquierdo, 2=derecho)
    const button = event.button === 0 ? 1 : 2;

    console.log(`Click en: (${realX}, ${realY}), botón: ${button}`);

    try {
      // Enviar click al backend
      await apiService.mouseClick(realX, realY, button, token);
    } catch (error) {
      console.error('Error al enviar click:', error);
    }
  };

  // Handler para presionar teclas
  const handleKeyDown = async (event) => {
    if (!canControl()) {
      console.log('No tienes permisos de control');
      return;
    }

    // Prevenir comportamiento por defecto del navegador
    event.preventDefault();

    const keycode = event.keyCode || event.which;
    console.log(`Tecla presionada: ${event.key} (keycode: ${keycode})`);

    try {
      // Enviar tecla al backend
      await apiService.keyPress(keycode, token);
    } catch (error) {
      console.error('Error al enviar tecla:', error);
    }
  };

  return (
    <div style={{ 
      display: 'flex', 
      flexDirection: 'column', 
      alignItems: 'center',
      padding: '20px'
    }}>
      {!screenshot ? (
        <div style={{ 
          width: canvasSize.width, 
          height: canvasSize.height,
          border: '2px dashed #ccc',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center'
        }}>
          <p>Esperando conexión...</p>
        </div>
      ) : (
        <canvas
          ref={canvasRef}
          width={canvasSize.width}
          height={canvasSize.height}
          onClick={handleCanvasClick}
          onContextMenu={(e) => {
            e.preventDefault();
            handleCanvasClick(e);
          }}
          onKeyDown={handleKeyDown}
          tabIndex="0" // Necesario para que el canvas pueda recibir eventos de teclado
          style={{
            border: '2px solid #333',
            cursor: canControl() ? 'crosshair' : 'default'
          }}
        />
      )}

      {/* Instrucciones */}
      <div style={{ marginTop: '15px', textAlign: 'center' }}>
        {canControl() ? (
          <p>Puedes controlar el escritorio remoto</p>
        ) : (
          <p>Solo puedes visualizar (sin permisos de control)</p>
        )}
        <small>Haz click en el canvas y usa tu teclado para interactuar</small>
      </div>
    </div>
  );
};

export default RemoteDesktop;