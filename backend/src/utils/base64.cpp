#include "utils/base64.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>

namespace Utils {

std::string base64Encode(const std::vector<unsigned char>& data) {
    if (data.empty()) return "";

    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    
    // No agregar saltos de línea
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    
    BUF_MEM* buffer = nullptr;
    BIO_get_mem_ptr(bio, &buffer);
    
    std::string result(buffer->data, buffer->length);
    
    BIO_free_all(bio);
    
    return result;
}

std::vector<unsigned char> base64Decode(const std::string& encoded) {
    if (encoded.empty()) return {};

    BIO* bio = BIO_new_mem_buf(encoded.data(), encoded.length());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    std::vector<unsigned char> result(encoded.length());
    int decoded_length = BIO_read(bio, result.data(), encoded.length());
    
    BIO_free_all(bio);
    
    result.resize(decoded_length > 0 ? decoded_length : 0);
    
    return result;
}

} // namespace Utils