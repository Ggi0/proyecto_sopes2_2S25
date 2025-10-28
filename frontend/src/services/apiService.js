
// URL base del backend 
const BASE_URL = 'http://10.150.1.233:8080';

// Función auxiliar para hacer peticiones HTTP
const request = async (endpoint, options = {}) => {
  const url = `${BASE_URL}${endpoint}`;
  
  try {
    const response = await fetch(url, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...options.headers,
      },
    });

    const data = await response.json();
    
    if (!response.ok) {
      throw new Error(data.error || 'Request failed');
    }
    
    return data;
  } catch (error) {
    console.error('API Error:', error);
    throw error;
  }
};

// Servicio de API con todos los endpoints
const apiService = {
  // ==================== AUTENTICACIÓN ====================
  
  // Login: recibe username y password, devuelve token y datos del usuario
  login: async (username, password) => {
    return request('/api/auth/login', {
      method: 'POST',
      body: JSON.stringify({ username, password }),
    });
  },

  // Logout: cierra sesión
  logout: async (token) => {
    return request('/api/auth/logout', {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token}`,
      },
    });
  },

  // ==================== CONTROL ====================
  
  // Health check del servidor
  health: async () => {
    return request('/health');
  },

  // Click del mouse: envía coordenadas x, y y botón (1=izquierdo, 2=derecho)
  mouseClick: async (x, y, button, token) => {
    return request('/api/mouse/click', {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token}`,
      },
      body: JSON.stringify({ x, y, button }),
    });
  },

  // Presionar tecla: envía el keycode de la tecla presionada
  keyPress: async (key, token) => {
    return request('/api/keyboard/press', {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token}`,
      },
      body: JSON.stringify({ key }),
    });
  },
};

export default apiService;