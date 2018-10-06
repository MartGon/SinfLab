#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

void aes_encrypt(const char Key[128], const char iv[128]);