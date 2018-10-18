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
#include <fstream>

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

// size - Key's size in bytes
unsigned char* generateRandomKey(int32_t size);

// Debug functions
void test_ciphering();

void log(std::string str);

// Useful classes
class Node
{
public:
	// Constructor
	Node();
	Node(Key key);

	// Node Utility variables
	int32_t id = 1;

	// Key
	Key key = nullptr;

	// Revoked
	bool revoked = false;

	// Methods
	bool isDevice(Tree tree);
	void revoke(Tree tree);

	// Tree handling
	Node* getParent(Tree tree);
	Node* getSibling(Tree tree);
	std::pair<Node*, Node*> getChildren(Tree tree);
private:
	// Id measures
	static int16_t last_id;
	int32_t get_id();
};

// Tree functions

int32_t calculateTreeSize(int32_t levels);

Tree generateBinaryTree(int32_t levels);

std::vector<Node*> getRevokedNodes(Tree tree);

std::vector<Node*>getValidKeyNodes(Tree tree, std::vector<Node*> revoked_nodes);

Tree updateTree(Tree tree, std::vector<int> revoked_devices);

// Key

struct KeyStruct
{
	int32_t key_id;
	Key ciphered_key;
};

// Header
struct Header
{
public:
	int32_t keys_size;
	int32_t key_array_length;
	std::vector<KeyStruct> ciphered_keys;
};

// Header Functions

Header* generateHeader(std::vector<Node*> valid_nodes, Key key);