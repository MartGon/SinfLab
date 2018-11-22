
// C++ standard libs
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

// OpenSSL libs

#include "openssl/sha.h"
#include <openssl/err.h>

// Forward Declaration
class Node;
struct BlockChainBlock;
struct BlockChainHeader;

// Var Utilities 
using hexstr = std::string;
using blockchain = std::vector<BlockChainBlock>;
using Tree = std::vector<Node*>;
using pattern = unsigned char[4];
using timestamp = unsigned char[4];
using Key = unsigned char*;

// Block Structure

struct BlockChainHeader
{
	uint32_t version;
	unsigned char hashPrevBlock[32];
	unsigned char hashMerkleTree[32];
	timestamp time;
	pattern bits;
	uint32_t nonce;
};

struct BlockChainBlock
{
	uint32_t magic_number = 0;
	uint32_t size = 0;
	BlockChainHeader header;
	unsigned char* payload;
};

// File Reading

template <typename T>
void readFromFile(std::fstream& file, T& var)
{
	file.read((char*)&var, sizeof(T));
	return;
}

BlockChainBlock* getNextBlock(std::fstream& blockchain_file, bool &eof);

bool seekMagicNumber(std::fstream& blockchain_file);

// Class Node

class Node
{
public:
	// Constructor
	Node();
	Node(uint32_t id);
	Node(BlockChainBlock* block, unsigned char* hash);

	// Node Utility variables
	int32_t id = 1;

	// Block
	BlockChainBlock* block = nullptr;

	// Hex hash
	unsigned char* hash = nullptr;

	// Methods
	hexstr getHexHash();

	// Tree handling
	Node* getParent(Tree tree);
	Node* getSibling(Tree tree);

	uint32_t getParentId(Tree tree);

	std::pair<Node*, Node*> getChildren(Tree tree);
	bool isBlock();

private:
	// Id measures
	static int16_t last_id;
	int32_t get_id();
};

// Tree handling

uint32_t calculateTreeSize(uint32_t devices);

// Label functions
// Traditional label -> Root = tree.size()
// Standard label -> Root = 1

int32_t tradLabelToStandard(int32_t id, uint32_t size);

// OpenSSL implementations

void sha256(unsigned char* input, int32_t input_length, unsigned char* output);

// Readablility

std::string ucharToString(unsigned char* str, int32_t str_length);

unsigned char* bitswap(unsigned char* in, uint32_t in_length);