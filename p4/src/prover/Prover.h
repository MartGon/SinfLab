
// C++ standard libs
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using pattern = unsigned char[4];
using timestamp = unsigned char[4];

struct BlockChainBlock;
struct BlockChainHeader;

struct Tree
{
	unsigned char* id;
	std::vector<BlockChainBlock> blocks;
};

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

BlockChainBlock getNextBlock(std::fstream& blockchain_file, bool &eof);

bool seekMagicNumber(std::fstream& blockchain_file);