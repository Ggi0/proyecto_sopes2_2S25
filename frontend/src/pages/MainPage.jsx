import { useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAuth } from '../hooks/useAuth';
import { useWebSocket } from '../hooks/useWebSocket';
import Header from '../components/Header';
import Footer from '../components/Footer';
import SystemStats from '../components/SystemStats';
import RemoteDesktop from '../components/RemoteDesktop';

const MainPage = () => {
  const { isAuthenticated, canView } = useAuth();
  const navigate = useNavigate();
  
  // Hook personalizado para WebSocket
  const { isConnected, screenshot, resources, connect, disconnect } = useWebSocket();

  // Verificar autenticación al montar el componente
  useEffect(() => {
    if (!isAuthenticated) {
      // Si no está autenticado, redirigir al login
      navigate('/login');
      return;
    }

    if (!canView()) {
      // Si no tiene permisos de visualización, mostrar error
      alert('No tienes permisos para ver el escritorio remoto');
      navigate('/login');
      return;
    }

    // Conectar al WebSocket
    connect();

    // Cleanup: desconectar al desmontar el componente
    return () => {
      disconnect();
    };
  }, [isAuthenticated, canView, navigate, connect, disconnect]);

  return (
    <div style={{
      display: 'flex',
      flexDirection: 'column',
      minHeight: '100vh'
    }}>
      {/* Header con información del usuario */}
      <Header />

      {/* Cuerpo principal */}
      <main style={{ flex: 1, padding: '20px' }}>
        {/* Indicador de conexión */}
        <div style={{ 
          padding: '10px', 
          marginBottom: '15px',
          background: isConnected ? '#e8f5e9' : '#ffebee',
          borderRadius: '5px',
          textAlign: 'center'
        }}>
          {isConnected ? ' Conectado al servidor' : ' Desconectado'}
        </div>

        {/* Estadísticas del sistema (CPU y RAM) */}
        <SystemStats resources={resources} />

        {/* Canvas con el escritorio remoto */}
        <RemoteDesktop screenshot={screenshot} />
      </main>

      {/* Footer */}
      <Footer />
    </div>
  );
};

export default MainPage;