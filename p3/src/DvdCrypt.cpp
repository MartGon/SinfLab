#include "DvdCrypt.h"


// Config
bool debug = true;
const int32_t KEY_SIZE = 16;
byte* iv = (byte*)"McQfTjWnZr4u7x!";

int main(void)
{
	// Initiliaze random generator to use a hardware source of entropy
	initRandomGenerator();

	// Debug
	// test_ciphering();

	// Get revoked devices' ids
	std::vector<int> revoked_devices = {8};

	// Generate the binary tree of 4 levels
	Tree tree = generateBinaryTree(4);

	// Update the tree with the revoked keys
	tree = updateTree(tree, revoked_devices);

	// Get revoked nodes
	std::vector<Node*> revoked_nodes = getRevokedNodes(tree);

	// Get the cover of S
	std::vector<Node*> valid_nodes = getValidKeyNodes(tree, revoked_nodes);

	// Generate the key for encrypting the content
	int32_t key_size = KEY_SIZE;
	Key key = generateRandomKey(key_size);

	// Ecnrypt the key with the keys from the covers of S
	// Generate the header of the file
	Header* header = generateHeader(valid_nodes, key);

	// Encrypt the content
	byte* content = (byte*)"Hola, hello, salut, guten";
	int32_t content_length = strlen((char*)content);
	byte* ciphertext = new byte[128];
	int32_t ciphertext_length = 0;
	ciphertext_length = aes_encrypt_func(content, content_length, key, iv, ciphertext);

	// Update header
	header->ciphered_content_length = ciphertext_length;
	header->content_length = content_length;

	// Debug
	/*
	byte* decrypted = new byte[128];
	aes_decrypt_func(ciphertext, ciphertext_length, key, iv, decrypted);
	decrypted[content_length] = '\0';
	*/

	// Write to file
	const char* filename = "cipher-content.cry";
	writeToFile(filename, header, ciphertext, ciphertext_length);
	std::cout << "Content has been successfully encrypted on " <<  filename << " \n";

	// DECRYPTION

	// Prompt for decryption
	Node* decrypting_node;
	int device = 1;

	while (true)
	{
		std::cout << "In order to decrypt, choose a device for decrypting\n";
		std::cin >> device;
		std::cin.get();
		std::cout << "You have chosen device " << std::to_string(device) << " \n";

		if (isValidIndex(tree, device))
		{
			Node* node = tree.at(device);
			if (node->isDevice(tree))
			{
				decrypting_node = node;
				break;
			}
			else
				std::cout << "The node with the given id is not a device\n";
		}
		else
			std::cout << "The node with the given id was not found\n";
	}

	// Log
	std::cout << "Trying to decrypt the file with device " << std::to_string(device) << "\n";

	// Read file
	Header read_header;
	byte* read_ciphered_content;
	readFromFile(filename, read_header, read_ciphered_content);
	
	// Discover if we can decrypt the content or not
	KeyStruct keystruct = decrypting_node->getDecryptKeyByHeader(tree, read_header);

	if (!keystruct.key_id)
	{
		std::cout << "Device cannot decrypt the content\n";
		std::cin.get();
		return -1;
	}

	// Decrypting process
	// Decrypting the key
	byte* key_to_decrypt = tree.at(keystruct.key_id)->key;
	byte* decrypted_key = new byte[keystruct.ciphered_key_size];
	byte* encrypted_key = keystruct.ciphered_key;
	int32_t encrypted_key_length = keystruct.ciphered_key_size;
	int32_t decrypted_key_size = read_header.keys_size;

	aes_decrypt_func(encrypted_key, encrypted_key_length, key_to_decrypt, iv, decrypted_key);
	decrypted_key[key_size] = '\0';

	// Decrypting the content
	byte* decrypted_content = new byte[header->content_length];
	aes_decrypt_func(read_ciphered_content, header->ciphered_content_length, decrypted_key, iv, decrypted_content);
	decrypted_content[header->content_length] = '\0';

	std::cout << "The decrypted content is \n\n" << decrypted_content << "\n";

	std::cin.get();

	return 0;
}

// Cipher fuctions

int32_t aes_encrypt_func(byte* plaintext, int32_t plaintext_len, const byte* key, const byte* iv, byte* ciphertext)
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
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
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

int32_t aes_decrypt_func(byte * ciphertext, int ciphertext_len, byte * key, byte * iv, byte * plaintext)
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

// Key generation
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

byte* generateRandomKey(int32_t size)
{
	byte* buffer = new byte[size];
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
	byte* key = nullptr;
	key = generateRandomKey(16);
	byte* message = (byte*)"Hola, hello, salut, guten";
	byte* ciphertext = new byte[128];
	byte decryptedtext[128];
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
int32_t calculateTreeSize(int32_t levels)
{
	return pow(2, levels) - 1;
}

Tree generateBinaryTree(int32_t levels)
{
	int32_t size = calculateTreeSize(levels);
	log("Tree size is " + std::to_string(size) + "\n");

	// Create Tree
	Tree tree;
	// Measure to keep indexing correctly
	tree.push_back(nullptr);

	// Create nodes
	for (int i = 0; i < size; i++)
	{
		Key key = generateRandomKey(KEY_SIZE);
		Node* node = new Node(key);
		tree.push_back(node);
	}

	return tree;
}

std::vector<Node*> getRevokedNodes(Tree tree)
{
	std::vector<Node*> nodes;

	for (int i = 0; i < tree.size(); i++)
	{
		if (Node* node = tree.at(i))
			if (node->revoked)
				nodes.push_back(node);
	}

	return nodes;
}

std::vector<Node*> getValidKeyNodes(Tree tree, std::vector<Node*> revoked_nodes)
{
	std::vector<Node*> valid_nodes;

	// If there are no revoked nodes, the root key can be used
	if (revoked_nodes.empty())
		valid_nodes.push_back(tree.at(1));

	// Get the first child from the revoked nodes which is not revoked
	for (int i = 0; i < revoked_nodes.size(); i++)
	{
		Node* node = revoked_nodes.at(i);

		std::pair<Node*, Node*> children = node->getChildren(tree);
		
		if(children.first)
			if (!children.first->revoked)
				valid_nodes.push_back(children.first);

		if (children.second)
			if (!children.second->revoked)
				valid_nodes.push_back(children.second);
	}

	return valid_nodes;
}

bool isValidIndex(Tree tree, int32_t index)
{
	return index > 0 && index < tree.size();
}

Tree updateTree(Tree tree, std::vector<int> revoked_devices)
{
	for (int i = 0; i < revoked_devices.size(); i++)
	{
		int index = revoked_devices.at(i);

		if (!isValidIndex(tree, index))
			continue;

		if (Node* node = tree.at(index))
			if (node->isDevice(tree))
				node->revoke(tree);
	}

	return tree;
}

// Classes
int16_t Node::last_id = 1;

// Constructors

Node::Node()
{
	id = get_id();
}

Node::Node(Key key) : Node::Node()
{
	this->key = key;
}

// Methods

// Public

KeyStruct Node::getDecryptKeyByHeader(Tree tree, Header header)
{
	// Get the keys which this devis has access to
	std::vector<int32_t> key_ids;
	Node* node = this;

	while (node)
	{
		key_ids.push_back(node->id);
		node = node->getParent(tree);
	}

	// Check whether there's a coincidence
	int32_t key_id = 0;
	for (int i = 0; i < header.ciphered_keys.size(); i++)
	{
		key_id = header.ciphered_keys.at(i).key_id;
		if (vector_contains(key_ids, key_id))
			return header.ciphered_keys.at(i);
	}

	return KeyStruct();
}

bool Node::isDevice(Tree tree)
{
	int32_t size = tree.size();

	// If it is from the last level
	if (id < size && id >= (size / 2))
		return true;

	return false;
}

void Node::revoke(Tree tree)
{
	revoked = true;
	Node* parent = getParent(tree);
	if (parent)
		parent->revoke(tree);
}

// Tree handling
Node* Node::getParent(Tree tree)
{
	if (id == 1)
		return nullptr;

	int parent_index = (id / 2);

	return tree.at(parent_index);
}

Node* Node::getSibling(Tree tree)
{
	if (id == 1)
		return nullptr;

	int sibling_index = 0;

	if (id % 2)
		sibling_index = (id - 1);
	else
		sibling_index = (id + 1);

	return tree.at(sibling_index);
}

std::pair<Node*, Node*> Node::getChildren(Tree tree)
{
	std::pair<Node*, Node*> children;
	int child_index = id * 2;

	if (child_index >= tree.size())
		return children;

	children.first = tree.at(child_index);
	children.second = tree.at(child_index + 1);

	return children;
}

// Private

int32_t Node::get_id()
{
	return last_id++;
}

// Header Functions

Header *generateHeader(std::vector<Node*> valid_nodes, Key key)
{
	std::vector<KeyStruct> ciphered_keys;
	Key ciphered_key = new byte[16];
	int32_t ciphered_key_size = 0;

	for (int i = 0; i < valid_nodes.size(); i++)
	{
		// Generate encrypted key
		Node* node = valid_nodes.at(i);
		Key key_to_cipher = node->key;
		ciphered_key = new byte[128];
		ciphered_key_size = aes_encrypt_func(key, KEY_SIZE, key_to_cipher, iv, ciphered_key);

		// Store result on a struct
		KeyStruct key_struct;
		key_struct.ciphered_key = ciphered_key;
		key_struct.ciphered_key_size = ciphered_key_size;
		key_struct.key_id = node->id;

		ciphered_keys.push_back(key_struct);
	}

	// Generate the file's header
	Header* header = new Header();
	header->ciphered_keys = ciphered_keys;
	header->keys_size = KEY_SIZE;
	header->key_array_length = ciphered_keys.size();

	return header;
}

// File I/O Functions
void writeToFile(const char* filename, Header* header, byte* content, int32_t content_length)
{
	// Open file
	std::fstream myfile;
	myfile = std::fstream(filename, std::ios::out | std::ios::binary);

	// Write header
	myfile.write((char*)&header->keys_size, sizeof(int32_t));
	myfile.write((char*)&header->key_array_length, sizeof(int32_t));
	myfile.write((char*)&header->content_length, sizeof(int32_t));
	myfile.write((char*)&header->ciphered_content_length, sizeof(int32_t));

	// Write keys with each id
	for (int i = 0; i < header->key_array_length; i++)
	{
		myfile.write((char*)&header->ciphered_keys.at(i).key_id, sizeof(int32_t));
		myfile.write((char*)&header->ciphered_keys.at(i).ciphered_key_size, sizeof(int32_t));
		myfile.write((char*)header->ciphered_keys.at(i).ciphered_key, header->ciphered_keys.at(i).ciphered_key_size);
	}

	// Write encrypted content
	myfile.write((char*)content, content_length);

	// Finish
	myfile.close();
}

int32_t readFromFile(const char* filename, Header& header, byte*& content)
{
	// Open file
	std::fstream myfile;
	myfile = std::fstream(filename, std::ios::in | std::ios::binary);

	// Read header
	myfile.read((char*)&header.keys_size, sizeof(int32_t));
	myfile.read((char*)&header.key_array_length, sizeof(int32_t));
	myfile.read((char*)&header.content_length, sizeof(int32_t));
	myfile.read((char*)&header.ciphered_content_length, sizeof(int32_t));

	// Read keys with each id
	for (int i = 0; i < header.key_array_length; i++)
	{
		KeyStruct keystruct;

		myfile.read((char*)&keystruct.key_id, sizeof(int32_t));
		myfile.read((char*)&keystruct.ciphered_key_size, sizeof(int32_t));
		keystruct.ciphered_key = new byte[keystruct.ciphered_key_size];
		myfile.read((char*)keystruct.ciphered_key, keystruct.ciphered_key_size);

		header.ciphered_keys.push_back(keystruct);
	}

	// Read content
	content = new byte[header.ciphered_content_length];
	myfile.read((char*)content, header.ciphered_content_length);

	// Return
	return header.content_length;
}

// Vector functions

bool vector_contains(std::vector<int32_t> vector, int32_t value)
{
	for (int i = 0; i < vector.size(); i++)
	{
		if (vector.at(i) == value)
			return true;
	}

	return false;
}
