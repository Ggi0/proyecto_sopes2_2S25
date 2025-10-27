// frontend/src/App.jsx
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { AuthProvider } from './contexts/AutoContext';
import LoginPage from './pages/LoginPage';
import RegisterPage from './pages/RegisterPage';
import MainPage from './pages/MainPage';

function App() {
  return (
    // Envolvemos toda la app con el AuthProvider para tener acceso al contexto de autenticación
    <AuthProvider>
      {/* Router para manejar las rutas */}
      <Router>
        <Routes>
          {/* Ruta del login */}
          <Route path="/login" element={<LoginPage />} />
          
          {/* Ruta de registro */}
          <Route path="/register" element={<RegisterPage />} />
          
          {/* Ruta principal (escritorio remoto) */}
          <Route path="/" element={<MainPage />} />
          
          {/* Ruta por defecto: redirigir a login */}
          <Route path="*" element={<Navigate to="/login" replace />} />
        </Routes>
      </Router>
    </AuthProvider>
  );
}

export default App;