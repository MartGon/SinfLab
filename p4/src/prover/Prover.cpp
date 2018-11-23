#include "Prover.h"

// Magic number
const uint32_t magic_number = 0xd9b4bef9;

int main(int arg, char* argv[])
{
	if (arg != 2)
	{
		std::cout << "Expected a just a filename with a blockchain dataset\n";
		return -1;
	}

	// Create Tree
	Tree tree;
	tree.push_back(nullptr);

	// Open file
	char* filename = argv[1];
	std::fstream myfile;
	myfile = std::fstream(filename, std::ios::in | std::ios::binary);

	// Read file
	bool eof = false;

	// Variables to use during loop
	BlockChainBlock* prev_block = nullptr;
	unsigned char* prev_block_hash;

	while (!eof)
	{
		// Get block
		BlockChainBlock* block;
		block = getNextBlock(myfile, eof);

		// Hash to string
		prev_block_hash = block->header.hashPrevBlock;

		// Insert block
		Node* node = nullptr;
		if (prev_block)
		{
			node = new Node(prev_block, prev_block_hash);
			tree.push_back(node);
		}

		// Set prev block
		prev_block = block;
	}

	// Add dummy block if odd number of blocks
	// -1 Due to the first dummy block inserted
	uint32_t blocks_amount = tree.size() - 1;

	// Expand tree size
	// +1 Due to the first dummy block inserted
	uint32_t tree_size = calculateTreeSize(blocks_amount) + 1;
	tree.resize(tree_size);

	// Relabel nodes
	for (uint32_t i = 1; i < blocks_amount; i++)
	{
		// Calculate new label
		Node* node = tree.at(i);
		uint32_t new_label = tradLabelToStandard(i, tree_size);

		// Set new label
		node->id = new_label;
		tree.at(new_label) = node;
		tree.at(i) = nullptr;
	}

	// Create upper nodes
	uint32_t end_index = blocks_amount;
	uint32_t start_index = 1;
	uint32_t diff = end_index - start_index;
	uint32_t quotient = 0;

	while (end_index != start_index)
	{
		for (uint32_t i = start_index; i < end_index; i += 2)
		{
			// Choose siblings
			uint32_t index1 = tradLabelToStandard(i, tree_size);
			uint32_t index2 = tradLabelToStandard(i + 1, tree_size);
			Node* n1 = tree.at(index1);
			Node* n2 = tree.at(index2);

			// Create parent
			uint32_t parent_id = n1->getParentId(tree);
			Node* node = new Node(parent_id);

			// Concat block hashes
			uint32_t concat_hash_length = SHA256_DIGEST_LENGTH * 2;
			unsigned char* concat_hash = new unsigned char[concat_hash_length];

			memcpy_s(concat_hash, concat_hash_length, n1->hash, SHA256_DIGEST_LENGTH);
			concat_hash += SHA256_DIGEST_LENGTH;
			memcpy_s(concat_hash, concat_hash_length, n2->hash, SHA256_DIGEST_LENGTH);
			concat_hash -= SHA256_DIGEST_LENGTH;

			// Compute hash
			unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
			sha256(concat_hash, concat_hash_length, hash);

			// Set hash
			node->hash = hash;

			// Set parent node in tree
			tree.at(parent_id) = node;
		}

		// Calculate new index
		diff = end_index - start_index;
		quotient = diff / 2;

		// Set new index
		start_index = diff & 1 ? end_index + 1 : end_index;
		end_index = start_index + quotient;
	}

	// Ask for user input
	std::cout << "Press enter to close the program\n";

	// Close file
	myfile.close();

	// Wait for user input
	std::cin.get();

	return 0;
}

BlockChainBlock* getNextBlock(std::fstream& blockchain_file, bool &eof)
{
	int32_t index = blockchain_file.tellg();

	// Create Block
	BlockChainBlock* block = new BlockChainBlock();

	// Seek magic number byte by byte if index == 0
	if (!index)
	{
		while (!seekMagicNumber(blockchain_file))
			blockchain_file.seekg(index++);
	}
	// Seek magic number otherwise
	else if (!seekMagicNumber(blockchain_file))
	{
		std::cout << "Magic number could not be read\n";
		eof = true;
		return block;
	}
	
	// Set magic number without reading again
	block->magic_number = magic_number;
	readFromFile(blockchain_file, block->size);

	// Read header
	BlockChainHeader header;

	readFromFile(blockchain_file, header.version);
	readFromFile(blockchain_file, header.hashPrevBlock);
	readFromFile(blockchain_file, header.hashMerkleTree);
	readFromFile(blockchain_file, header.time);
	readFromFile(blockchain_file, header.bits);
	readFromFile(blockchain_file, header.nonce);

	block->header = header;

	// Read payload
	int32_t read_bytes = (int)blockchain_file.tellg() - index;
	int32_t left_bytes = block->size - read_bytes;
	block->payload = new unsigned char[left_bytes];
	blockchain_file.read((char*)block->payload, left_bytes);

	// Set eof
	eof = blockchain_file.eof();

	// Move file index
	blockchain_file.seekg((int32_t)blockchain_file.tellg() + 8);

	return block;
}

bool seekMagicNumber(std::fstream& blockchain_file)
{
	uint32_t magic_candidate;
	readFromFile(blockchain_file, magic_candidate);

	return magic_candidate == magic_number;
}

void sha256(unsigned char *input, int32_t input_length, unsigned char* output)
{
	// Declare hashing object
	SHA256_CTX sha256;

	char* error_msg = new char[32];
	if (!SHA256_Init(&sha256))
	{
		std::cout << "Error while intitializing hashing object\n";
		return;
	}

	if (!SHA256_Update(&sha256, input, input_length))
	{
		ERR_error_string(ERR_get_error(), error_msg);
		std::cout << "Error during update: " << std::string(error_msg) << "\n";
		return;
	}

	if (!SHA256_Final(output, &sha256))
	{
		std::cout << "Error finalizing\n";
		return;
	}

	delete error_msg;

	return;
}

// Classes
int16_t Node::last_id = 1;

// Constructors
Node::Node()
{
	id = get_id();
}

Node::Node(uint32_t id)
{
	this->id = id;
}

Node::Node(BlockChainBlock* block, unsigned char* hash) : Node::Node()
{
	this->block = block;
	this->hash = hash;
}

// Methods

hexstr Node::getHexHash()
{
	return ucharToString(bitswap(hash, SHA256_DIGEST_LENGTH), SHA256_DIGEST_LENGTH);
}

// Tree handling
uint32_t calculateTreeSize(uint32_t devices)
{
	return (devices * 2) - 1;
}

int32_t tradLabelToStandard(int32_t id, uint32_t size)
{
	return size - id;
}

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

	// Before id % 2
	if (id & 1)
		sibling_index = (id - 1);
	else
		sibling_index = (id + 1);

	return tree.at(sibling_index);
}

uint32_t Node::getParentId(Tree tree)
{
	if (id == 1)
		return 0;

	uint32_t size = tree.size();

	int parent_index = (id / 2);

	return parent_index;
}

std::pair<Node*, Node*> Node::getChildren(Tree tree)
{
	std::pair<Node*, Node*> children;
	unsigned int child_index = id * 2;

	if (child_index >= tree.size())
		return children;

	children.first = tree.at(child_index);
	children.second = tree.at(child_index + 1);

	return children;
}

bool Node::isBlock() 
{
	return block;
}

// Private

int32_t Node::get_id()
{
	return last_id++;
}

// Readability

std::string ucharToString(unsigned char * str, int32_t str_length)
{
	std::stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)str[i];
	}
	return ss.str();
}

unsigned char* bitswap(unsigned char* in, uint32_t in_length)
{
	unsigned char* output = new unsigned char[in_length];
	
	if (in_length & 1)
	{
		std::cout << "The input was not even\n";
		delete output;
		return nullptr;
	}

	for (uint32_t i = 0; i < in_length; i++)
	{
		uint32_t opp = in_length - i - 1;
		unsigned char b = in[i];
		output[opp] = b;
	}

	return output;
}