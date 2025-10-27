import { createContext, useState, useEffect } from 'react';

// Creamos el contexto de autenticación que será accesible en toda la app
export const AuthContext = createContext();

export const AuthProvider = ({ children }) => {
  // Estado para almacenar la información del usuario logueado
  const [user, setUser] = useState(null);
  const [token, setToken] = useState(null);
  const [isAuthenticated, setIsAuthenticated] = useState(false);

  // Al montar el componente, verificamos si hay una sesión guardada en localStorage
  useEffect(() => {
    const savedToken = localStorage.getItem('token');
    const savedUser = localStorage.getItem('user');
    
    if (savedToken && savedUser) {
      setToken(savedToken);
      setUser(JSON.parse(savedUser));
      setIsAuthenticated(true);
    }
  }, []);

  // Función para hacer login
  const login = (userData, userToken) => {
    setUser(userData);
    setToken(userToken);
    setIsAuthenticated(true);
    
    // Guardamos en localStorage para persistir la sesión
    localStorage.setItem('token', userToken);
    localStorage.setItem('user', JSON.stringify(userData));
  };

  // Función para hacer logout
  const logout = () => {
    setUser(null);
    setToken(null);
    setIsAuthenticated(false);
    
    // Limpiamos localStorage
    localStorage.removeItem('token');
    localStorage.removeItem('user');
  };

  // Verificar si el usuario tiene permiso de visualización
  const canView = () => {
    return user?.can_view || false;
  };

  // Verificar si el usuario tiene permiso de control
  const canControl = () => {
    return user?.can_control || false;
  };

  // Proporcionamos todos estos valores y funciones a través del contexto
  const value = {
    user,
    token,
    isAuthenticated,
    login,
    logout,
    canView,
    canControl
  };

  return (
    <AuthContext.Provider value={value}>
      {children}
    </AuthContext.Provider>
  );
};