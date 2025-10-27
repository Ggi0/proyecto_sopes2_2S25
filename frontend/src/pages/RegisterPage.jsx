import { useState } from 'react';
import { useNavigate } from 'react-router-dom';

const RegisterPage = () => {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [error, setError] = useState('');
  const [success, setSuccess] = useState(false);
  
  const navigate = useNavigate();

  const handleSubmit = async (e) => {
    e.preventDefault();
    setError('');
    setSuccess(false);

    // Validaciones básicas
    if (password !== confirmPassword) {
      setError('Las contraseñas no coinciden');
      return;
    }

    if (password.length < 3) {
      setError('La contraseña debe tener al menos 3 caracteres');
      return;
    }

    try {
      // TODO: Implementar endpoint de registro en el backend
      // Por ahora solo mostramos mensaje
      console.log('Registro solicitado:', { username, password });
      
      setSuccess(true);
      
      // Redirigir al login después de 2 segundos
      setTimeout(() => {
        navigate('/login');
      }, 2000);
      
    } catch (err) {
      setError(err.message || 'Error al registrarse');
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
          📝 Crear Cuenta
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

          {/* Confirmar contraseña */}
          <div style={{ marginBottom: '20px' }}>
            <label htmlFor="confirmPassword">Confirmar Contraseña:</label>
            <input
              type="password"
              id="confirmPassword"
              value={confirmPassword}
              onChange={(e) => setConfirmPassword(e.target.value)}
              required
              style={{
                width: '100%',
                padding: '10px',
                marginTop: '5px',
                fontSize: '16px'
              }}
            />
          </div>

          {/* Mensajes */}
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

          {success && (
            <div style={{
              padding: '10px',
              marginBottom: '15px',
              background: '#e8f5e9',
              color: '#2e7d32',
              borderRadius: '5px'
            }}>
              ¡Registro exitoso! Redirigiendo al login...
            </div>
          )}

          {/* Botón de registro */}
          <button
            type="submit"
            style={{
              width: '100%',
              padding: '12px',
              fontSize: '16px',
              cursor: 'pointer'
            }}
          >
            Registrarse
          </button>
        </form>

        {/* Link a login */}
        <div style={{ marginTop: '20px', textAlign: 'center' }}>
          <p>
            ¿Ya tienes cuenta?{' '}
            <a href="/login" style={{ color: '#1976d2' }}>
              Inicia sesión aquí
            </a>
          </p>
        </div>
      </div>
    </div>
  );
};

export default RegisterPage;