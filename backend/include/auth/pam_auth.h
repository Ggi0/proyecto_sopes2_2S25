#ifndef PAM_AUTH_H
#define PAM_AUTH_H

#include <string>
#include <vector>
#include "../types.h"

namespace Auth {

/**
 * @brief Autentica un usuario usando PAM
 * 
 * @param username Nombre de usuario del sistema Linux
 * @param password Contraseña del usuario
 * @return true si la autenticación fue exitosa, false en caso contrario
 */
bool authenticateUser(const std::string& username, const std::string& password);

/**
 * @brief Obtiene la lista de grupos a los que pertenece un usuario
 * 
 * @param username Nombre de usuario
 * @return std::vector<std::string> Lista de nombres de grupos
 */
std::vector<std::string> getUserGroups(const std::string& username);

/**
 * @brief Determina el nivel de acceso basado en los grupos del usuario
 * 
 * @param groups Lista de grupos del usuario
 * @return AccessLevel Nivel de acceso (NONE, VIEW_ONLY, FULL_CONTROL)
 */
AccessLevel determineAccessLevel(const std::vector<std::string>& groups);

/**
 * @brief Verifica si un usuario pertenece a un grupo específico
 * 
 * @param username Nombre de usuario
 * @param groupname Nombre del grupo
 * @return true si el usuario pertenece al grupo, false en caso contrario
 */
bool userInGroup(const std::string& username, const std::string& groupname);

/**
 * @brief Genera un token JWT simple para mantener la sesión
 * 
 * @param username Nombre de usuario
 * @return std::string Token generado
 */
std::string generateToken(const std::string& username);

/**
 * @brief Valida un token JWT
 * 
 * @param token Token a validar
 * @return std::string Nombre de usuario si es válido, cadena vacía si no
 */
std::string validateToken(const std::string& token);

} // namespace Auth

#endif // PAM_AUTH_H