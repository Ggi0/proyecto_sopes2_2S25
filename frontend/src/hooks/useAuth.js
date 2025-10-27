// frontend/src/hooks/useAuth.js
import { useContext } from 'react';
import { AuthContext } from '../contexts/AutoContext';

// Hook personalizado para acceder fácilmente al contexto de autenticación
export const useAuth = () => {
  const context = useContext(AuthContext);
  
  if (!context) {
    throw new Error('useAuth debe usarse dentro de un AuthProvider');
  }
  
  return context;
};