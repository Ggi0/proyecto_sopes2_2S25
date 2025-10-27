import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAuth } from '../hooks/useAuth';
import apiService from '../services/apiService';

const LoginPage = () => {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  
  const { login } = useAuth();
  const navigate = useNavigate();

  const handleSubmit = async (e) => {
    e.preventDefault();
    setError('');
    setLoading(true);

    try {
      // Llamar al endpoint de login
      const response = await apiService.login(username, password);
      
      if (response.success) {
        // Guardar datos del usuario en el contexto
        login({
          username: response.username,
          access_level: response.access_level,
          groups: response.groups,
          can_view: response.can_view,
          can_control: response.can_control,
        }, response.token);

        // Redirigir a la página principal
        navigate('/');
      }
    } catch (err) {
      setError(err.message || 'Error al iniciar sesión');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div style={{
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      minHeight: '100vh',
      padding: '20px'
    }}>
      <div style={{
        maxWidth: '400px',
        width: '100%',
        padding: '40px',
        border: '2px solid #333',
        borderRadius: '10px'
      }}>
        <h2 style={{ textAlign: 'center', marginBottom: '30px' }}>
          🖥️ USAC Linux Remote Desktop
        </h2>

        <form onSubmit={handleSubmit}>
          {/* Usuario */}
          <div style={{ marginBottom: '20px' }}>
            <label htmlFor="username">Usuario:</label>
            <input
              type="text"
              id="username"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              required
              style={{
                width: '100%',
                padding: '10px',
                marginTop: '5px',
                fontSize: '16px'
              }}
            />
          </div>

          {/* Contraseña */}
          <div style={{ marginBottom: '20px' }}>
            <label htmlFor="password">Contraseña:</label>
            <input
              type="password"
              id="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              required
              style={{
                width: '100%',
                padding: '10px',
                marginTop: '5px',
                fontSize: '16px'
              }}
            />
          </div>

          {/* Mensaje de error */}
          {error && (
            <div style={{
              padding: '10px',
              marginBottom: '15px',
              background: '#ffebee',
              color: '#c62828',
              borderRadius: '5px'
            }}>
              {error}
            </div>
          )}

          {/* Botón de login */}
          <button
            type="submit"
            disabled={loading}
            style={{
              width: '100%',
              padding: '12px',
              fontSize: '16px',
              cursor: loading ? 'not-allowed' : 'pointer'
            }}
          >
            {loading ? 'Iniciando sesión...' : 'Iniciar Sesión'}
          </button>
        </form>

        {/* Link a registro */}
        <div style={{ marginTop: '20px', textAlign: 'center' }}>
          <p>
            ¿No tienes cuenta?{' '}
            <a href="/register" style={{ color: '#1976d2' }}>
              Regístrate aquí
            </a>
          </p>
        </div>
      </div>
    </div>
  );
};

export default LoginPage;