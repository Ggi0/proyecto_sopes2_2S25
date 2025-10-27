const SystemStats = ({ resources }) => {
  // Función para formatear bytes a MB
  const formatMB = (mb) => {
    return (mb / 1024).toFixed(2);
  };

  return (
    <div style={{
      padding: '15px',
      border: '1px solid #333',
      borderRadius: '5px',
      marginBottom: '10px',
      display: 'flex',
      justifyContent: 'space-around',
      gap: '20px'
    }}>
      {/* CPU */}
      <div>
        <strong>CPU:</strong> {resources.cpu}%
        <div style={{
          width: '200px',
          height: '20px',
          background: '#ddd',
          borderRadius: '5px',
          overflow: 'hidden'
        }}>
          <div style={{
            width: `${resources.cpu}%`,
            height: '100%',
            background: resources.cpu > 80 ? 'red' : resources.cpu > 50 ? 'orange' : 'green',
            transition: 'width 0.3s'
          }}></div>
        </div>
      </div>

      {/* RAM */}
      <div>
        <strong>RAM:</strong> {resources.ram}%
        <div>
          <small>
            {formatMB(resources.ram_total)} GB / {formatMB(resources.ram_free)} GB
          </small>
        </div>
        <div style={{
          width: '200px',
          height: '20px',
          background: '#ddd',
          borderRadius: '5px',
          overflow: 'hidden'
        }}>
          <div style={{
            width: `${resources.ram}%`,
            height: '100%',
            background: resources.ram > 80 ? 'red' : resources.ram > 50 ? 'orange' : 'green',
            transition: 'width 0.3s'
          }}></div>
        </div>
      </div>
    </div>
  );
};

export default SystemStats;