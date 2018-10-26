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
struct Header;
struct KeyStruct;

// Menu functions

int32_t encryptContent(int32_t nodes, char* content_filename, char* revokedset);

int32_t decryptContent(char* ciphered_content_filename, int32_t device);

// Utilities
using Tree = std::vector<Node*>;
using Key = byte*;

// Cipher fuctions
int32_t aes_encrypt_func(byte* plaintext, int32_t plaintext_len, const byte* key, const byte* iv, byte* ciphertext);

int32_t aes_decrypt_func(byte *ciphertext, int ciphertext_len, byte *key, byte *iv, byte *plaintext);

// Key generation

void initRandomGenerator();

// size - Key's size in bytes
byte* generateRandomKey(int32_t size);

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
	KeyStruct getDecryptKeyByHeader(Tree tree, Header header);
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

int32_t calculateTreeSize(int32_t devices);

Tree generateBinaryTree(int32_t devices);

std::vector<Node*> getRevokedNodes(Tree tree);

std::vector<int> getRevokedNodesFromArgs(char* revokedset);

std::vector<Node*>getValidKeyNodes(Tree tree, std::vector<Node*> revoked_nodes);

bool isValidIndex(Tree tree, int32_t index);

Tree updateTree(Tree tree, std::vector<int> revoked_devices);

// Label functions
// Traditional label -> Root = tree.size()
// Standard label -> Root = 1

int32_t tradLabelToStandard(int32_t id, Tree tree);

int32_t standardLabelToTrad(int32_t id, Tree tree);

// Key

struct KeyStruct
{
	int32_t key_id;
	int32_t ciphered_key_size;
	Key ciphered_key;
};

// Header
struct Header
{
public:
	int32_t keys_size;
	int32_t key_array_length;
	int32_t content_length;
	int32_t ciphered_content_length;
	std::vector<KeyStruct> ciphered_keys;
};

// Header Functions

Header* generateHeader(std::vector<Node*> valid_nodes, Key key);

// File I/O Functions

int32_t readFromFile(const char* filename, Header& header, byte*& content);

int readContentFromFile(const char* filename, byte*& buffer);

Tree readKeysFile();

void writeToFile(const char* filename, Header* header, byte* content, int32_t content_length);

void writeToContentFile(const char* filename, byte* buffer, int32_t content_length);

void writeKeysFile(Tree tree);

// Vector functions

bool vector_contains(std::vector<int32_t> vector, int32_t value);