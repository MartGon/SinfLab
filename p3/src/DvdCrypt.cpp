#include "DvdCrypt.h"

int main(void)
{


	return 0;
}

void aes_encrypt(const unsigned char key[128], const unsigned char iv[128])
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	int rc = EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

}