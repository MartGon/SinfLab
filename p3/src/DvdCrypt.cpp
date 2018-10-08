#include "DvdCrypt.h"

int main(void)
{
	unsigned char* key = (unsigned char*)"*F)J@NcRfUjXn2r";
	unsigned char* iv = (unsigned char*)"McQfTjWnZr4u7x!";
	unsigned char* message = (unsigned char*)"HolaHolaHola123";
	unsigned char* ciphertext = new unsigned char[128];
	unsigned char decryptedtext[128];
	int ciphertext_len = 0;
	int decryptedtext_len = 0;

	ciphertext_len = aes_encrypt_func(key, iv, message, strlen((char*)message), ciphertext);

	std::cout << (char*)ciphertext << "\n";

	decryptedtext_len = aes_decrypt_func(ciphertext, ciphertext_len, key, iv, decryptedtext);

	decryptedtext[decryptedtext_len] = '\0';

	std::cout << (char*)decryptedtext << "\n";

	getchar();

	return 0;
}

int32_t aes_encrypt_func(const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int32_t plaintext_len, unsigned char* ciphertext)
{
	// We create the encrypting object
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	// Create some variables to store length values after and before encrypting
	int32_t len;
	int32_t ciphertext_len;

	// Initialize it with AES-128-CBC
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
		std::cout << "Error during initialization";

	// Start encryptping
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, 16))
		std::cout << "Error in encrypting\n";
	ciphertext_len = len;

	// Finalize the encryption
	if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
		std::cout << "Error in finalizing";
	ciphertext_len += len;

	// Clean the encrypting object
	EVP_CIPHER_CTX_free(ctx);

	return ciphertext_len;
}

int32_t aes_decrypt_func(unsigned char * ciphertext, int ciphertext_len, unsigned char * key, unsigned char * iv, unsigned char * plaintext)
{
	// We create the encrypting object
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	// Create some variables to store length values after and before encrypting
	int32_t len;
	int32_t plaintext_len;

	// Initialize it with AES-128-CBC Decrypt operation
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
		std::cout << "Error during initialization";

	// Start decryptping
	if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		std::cout << "Error in encrypting\n";
	plaintext_len = len;

	// Finalize the encryption
	if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
		std::cout << "Error in finalizing";
	plaintext_len += len;

	// Clean the encrypting object
	EVP_CIPHER_CTX_free(ctx);

	return plaintext_len;
}