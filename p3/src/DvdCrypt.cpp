#include "DvdCrypt.h"


// Config
bool debug = true;

int main(void)
{
	initRandomGenerator();

	// Debug ciphering functions
	test_ciphering();

	generateBinaryTree(4);

	getchar();

	return 0;
}

// Cipher fuctions

int32_t aes_encrypt_func(unsigned char* plaintext, int32_t plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char* ciphertext)
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

void initRandomGenerator()
{
	// Variables
	unsigned long err = 0;
	int rc = 0;

	// Load engines
	ENGINE_load_builtin_engines();

	// Get the random generator engine
	ENGINE* eng = ENGINE_by_id("rdrand");

	if (!eng)
		log("Error while loading the random generator engine \n");

	// Initializing this engine
	rc = ENGINE_init(eng);
	err = ERR_get_error();

	if (!rc)
		std::cout << "ENGINE_init failed " + std::to_string(err) + "\n";

	// Setting default method rand
	rc = ENGINE_set_default(eng, ENGINE_METHOD_RAND);
	err = ERR_get_error();

	if (!rc)
		std::cout << "ENGINE_set_default failed " + std::to_string(err) + "\n";

	return;
}

// Key generation
unsigned char* generateRandomKey(int32_t size)
{
	unsigned char* buffer = new unsigned char[size];
	int rc = RAND_bytes(buffer, size);

	if (rc != 1)
		log("Error while generating a random key");

	// Cut char string
	buffer[size] = '\0';

	return buffer;
}

// Debug
void test_ciphering()
{
	unsigned char* key = (unsigned char*)"*F)J@NcRfUjXn2r";
	key = generateRandomKey(16);
	unsigned char* iv = (unsigned char*)"McQfTjWnZr4u7x!";
	unsigned char* message = (unsigned char*)"HolaHolaHola123";
	unsigned char* ciphertext = new unsigned char[128];
	unsigned char decryptedtext[128];
	int ciphertext_len = 0;
	int decryptedtext_len = 0;

	ciphertext_len = aes_encrypt_func(message, strlen((char*)message), key, iv, ciphertext);
	ciphertext[ciphertext_len] = '\0';

	std::cout << (char*)ciphertext << "\n";

	decryptedtext_len = aes_decrypt_func(ciphertext, ciphertext_len, key, iv, decryptedtext);

	decryptedtext[decryptedtext_len] = '\0';

	std::cout << (char*)decryptedtext << "\n";
}

void log(std::string str)
{
	if (debug)
		std::cout << str.c_str();
}

// Tree functions
Tree generateBinaryTree(int32_t levels)
{
	int32_t size = calculateTreeSize(levels);
	log("Tree size is " + std::to_string(size) + "\n");

	return Tree();
}

int32_t calculateTreeSize(int32_t levels)
{
	return pow(2, levels) - 1;
}

// Classes
int16_t Node::last_id = 0;

Node::Node(Node* parent, Node* sibling, Key key)
{
	id = get_id();
	this->parent = parent;
	this->sibling = sibling;
	this->key = key;
}

int32_t Node::get_id()
{
	return last_id++;
}
