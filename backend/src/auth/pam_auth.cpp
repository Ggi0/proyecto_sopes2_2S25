#include "auth/pam_auth.h"
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

#include <algorithm>

namespace Auth {

// Estructura para pasar credenciales a PAM
struct pam_response_data {
    std::string username;
    std::string password;
};

// Función de conversación requerida por PAM
static int pam_conversation(int num_msg, const struct pam_message **msg,
                           struct pam_response **resp, void *appdata_ptr) {
    
    pam_response_data* data = static_cast<pam_response_data*>(appdata_ptr);
    
    *resp = (struct pam_response*)calloc(num_msg, sizeof(struct pam_response));
    if (*resp == nullptr) {
        return PAM_BUF_ERR;
    }
    
    for (int i = 0; i < num_msg; i++) {
        switch (msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_ON:  // Username
                (*resp)[i].resp = strdup(data->username.c_str());
                (*resp)[i].resp_retcode = 0;
                break;
                
            case PAM_PROMPT_ECHO_OFF:  // Password
                (*resp)[i].resp = strdup(data->password.c_str());
                (*resp)[i].resp_retcode = 0;
                break;
                
            case PAM_ERROR_MSG:
                std::cerr << "PAM Error: " << msg[i]->msg << std::endl;
                (*resp)[i].resp = nullptr;
                (*resp)[i].resp_retcode = 0;
                break;
                
            case PAM_TEXT_INFO:
                std::cout << "PAM Info: " << msg[i]->msg << std::endl;
                (*resp)[i].resp = nullptr;
                (*resp)[i].resp_retcode = 0;
                break;
                
            default:
                return PAM_CONV_ERR;
        }
    }
    
    return PAM_SUCCESS;
}

bool authenticateUser(const std::string& username, const std::string& password) {
    pam_handle_t* pamh = nullptr;
    int retval;
    
    // Preparar datos para la conversación PAM
    pam_response_data conv_data;
    conv_data.username = username;
    conv_data.password = password;
    
    // Configurar estructura de conversación
    struct pam_conv conv = {
        pam_conversation,
        &conv_data
    };
    
    // Iniciar PAM
    retval = pam_start("login", username.c_str(), &conv, &pamh);
    if (retval != PAM_SUCCESS) {
        std::cerr << " Error al iniciar PAM: " << pam_strerror(pamh, retval) << std::endl;
        return false;
    }
    
    // Autenticar usuario
    retval = pam_authenticate(pamh, 0);
    if (retval != PAM_SUCCESS) {
        std::cerr << " Autenticación fallida para usuario '" << username 
                  << "': " << pam_strerror(pamh, retval) << std::endl;
        pam_end(pamh, retval);
        return false;
    }
    
    // Verificar que la cuenta sea válida (no expirada, etc.)
    retval = pam_acct_mgmt(pamh, 0);
    if (retval != PAM_SUCCESS) {
        std::cerr << " Cuenta inválida para usuario '" << username 
                  << "': " << pam_strerror(pamh, retval) << std::endl;
        pam_end(pamh, retval);
        return false;
    }
    
    // Finalizar PAM
    pam_end(pamh, PAM_SUCCESS);
    
    std::cout << "Usuario '" << username << "' autenticado exitosamente" << std::endl;
    return true;
}

std::vector<std::string> getUserGroups(const std::string& username) {
    std::vector<std::string> groups;
    
    // Obtener información del usuario
    struct passwd* pwd = getpwnam(username.c_str());
    if (pwd == nullptr) {
        std::cerr << " Usuario '" << username << "' no encontrado" << std::endl;
        return groups;
    }
    
    // Obtener grupo primario
    struct group* grp = getgrgid(pwd->pw_gid);
    if (grp != nullptr) {
        groups.push_back(grp->gr_name);
    }
    
    // Obtener grupos suplementarios
    int ngroups = 0;
    getgrouplist(username.c_str(), pwd->pw_gid, nullptr, &ngroups);
    
    if (ngroups > 0) {
        std::vector<gid_t> gids(ngroups);
        getgrouplist(username.c_str(), pwd->pw_gid, gids.data(), &ngroups);
        
        for (int i = 0; i < ngroups; i++) {
            grp = getgrgid(gids[i]);
            if (grp != nullptr) {
                std::string groupname = grp->gr_name;
                // Evitar duplicados
                if (std::find(groups.begin(), groups.end(), groupname) == groups.end()) {
                    groups.push_back(groupname);
                }
            }
        }
    }
    
    return groups;
}

AccessLevel determineAccessLevel(const std::vector<std::string>& groups) {
    // Verificar si tiene acceso de control completo
    for (const auto& group : groups) {
        if (group == Config::GROUP_CONTROL) {
            return AccessLevel::FULL_CONTROL;
        }
    }
    
    // Verificar si tiene acceso solo de visualización
    for (const auto& group : groups) {
        if (group == Config::GROUP_VIEW) {
            return AccessLevel::VIEW_ONLY;
        }
    }
    
    // Sin acceso
    return AccessLevel::NONE;
}

bool userInGroup(const std::string& username, const std::string& groupname) {
    std::vector<std::string> groups = getUserGroups(username);
    return std::find(groups.begin(), groups.end(), groupname) != groups.end();
}

std::string generateToken(const std::string& username) {
    // Token simple: SHA256(username + timestamp + salt)
    // En producción, usa JWT real con librerías como jwt-cpp
    
    std::string salt = "usac_remote_desktop_secret_key";
    std::string data = username + std::to_string(time(nullptr)) + salt;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

std::string validateToken(const std::string& token) {
    // En esta implementación simple, solo verificamos que el token no esté vacío
    // En producción, deberías validar y decodificar un JWT real
    
    if (token.empty() || token.length() != 64) {
        return "";
    }
    
    // Aquí deberías extraer el username del token
    // Por simplicidad, retornamos vacío (implementación básica)
    // En producción, usa JWT con claims
    
    return "";  // Implementar decodificación JWT real
}

} // namespace Auth