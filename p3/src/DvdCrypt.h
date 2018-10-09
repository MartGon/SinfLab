// OpenSSL
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/rand.h>

// C Standard Libs
#include <string.h>
#include <stdio.h>

// C++ Standard Libs
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <string>

// Forward Declaration
class Node;

// Utilities
using Tree = std::vector<Node*>;
using Key = unsigned char*;

// Cipher fuctions
int32_t aes_encrypt_func(unsigned char* plaintext, int32_t plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char* ciphertext);

int32_t aes_decrypt_func(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);

// Key generation
void initRandomGenerator();

unsigned char* generateRandomKey(int32_t size);

// Debug functions
void test_ciphering();

void log(std::string str);

// Useful classes
class Node
{
public:
	// Constructor
	Node(Node* parent, Node* sibling, Key key);

	// Node Utility variables
	int32_t id = 0;
	Node* parent = nullptr;
	Node* sibling = nullptr;
	std::pair<Node*, Node*> children;

	// Key
	Key key = nullptr;

private:
	// Id measures
	static int16_t last_id;
	int32_t get_id();
};

// Tree functions
Tree generateBinaryTree(int32_t levels);

int32_t calculateTreeSize(int32_t levels);