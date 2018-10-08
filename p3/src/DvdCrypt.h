#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <iostream>
#include <stdio.h>

int32_t aes_encrypt_func(const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int32_t plaintext_len, unsigned char* ciphertext);

int32_t aes_decrypt_func(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);