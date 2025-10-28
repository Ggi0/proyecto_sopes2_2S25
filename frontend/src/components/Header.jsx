import { useAuth } from '../hooks/useAuth';
import { useNavigate } from 'react-router-dom';

const Header = () => {
  const { isAuthenticated, user, logout } = useAuth();
  const navigate = useNavigate();

  const handleLogout = () => {
    logout();
    navigate('/login');
  };

  const handleRegistration = () => {
    logout();
    navigate('/register');
  };

  return (
    <header style={{ 
      padding: '20px', 
      borderBottom: '2px solid #333',
      display: 'flex',
      justifyContent: 'space-between',
      alignItems: 'center'
    }}>
      {/* Título de la aplicación */}
      <div>
        <h1> USAC Linux Remote Desktop</h1>
      </div>

      {/* Sección de usuario y botones */}
      <div style={{ display: 'flex', gap: '15px', alignItems: 'center' }}>
        {isAuthenticated ? (
          <>
            {/* Mostrar información del usuario logueado */}
            <div>
              <p><strong>{user?.username}</strong></p>
              <p style={{ fontSize: '12px', color: '#666' }}>
                Acceso: {user?.access_level}
              </p>
            </div>
            
            {/* Botón de logout */}
            <button onClick={handleLogout}>
              Cerrar Sesión
            </button>

            <button onClick={handleRegistration}>
              Registrar Usuario
            </button>
          </>
        ) : (
          <>
            {/* Botones para usuarios no autenticados */}
            <button onClick={() => navigate('/login')}>
              Iniciar Sesión
            </button>
            <button onClick={() => navigate('/register')}>
              Registrar Usr
            </button>
          </>
        )}
      </div>
    </header>
  );
};

export default Header;