#include "Prover.h"

// Magic number
const uint32_t magic_number = 0xd9b4bef9;
Tree tree;

int main(int arg, char* argv[])
{
	if (arg != 3)
	{
		std::cout << "Expected a filename with a blockchain dataset and a listening UDP port\n";
		return -1;
	}

	// Create Tree
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

			//if (node->id == 2047)
				//break;
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

	std::cout << "Tree creation progress 00%";
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
			unsigned char* hash = sumHash(n1->hash, n2->hash);

			// Set hash
			node->hash = hash;

			// Set parent node in tree
			tree.at(parent_id) = node;

			std::cout << "\rTree creation progress " << (int)((float)i / (float)tree_size * 100) << "%";
		}

		// Calculate new index
		diff = end_index - start_index;
		quotient = diff / 2;

		// Set new index
		start_index = diff & 1 ? end_index + 1 : end_index;
		end_index = start_index + quotient;
	}

	std::cout << std::endl;

	// Verify proccess

	// Init libs
	initNetworkingLibs();

	// Get port from arguments
	Uint16 port = std::stoi(argv[2]);

	// Open UDP sockets
	UDPsocket udp_socket;

	udp_socket = SDLNet_UDP_Open(port);

	if (!udp_socket)
	{
		std::cout << "Error while opening udp socket at port " << port << "\n";
		return -1;
	}

	// Listen to recieving packets
	initProverServer(udp_socket);

	// Close socket
	SDLNet_UDP_Close(udp_socket);

	// Shutdown  networking libs
	SDLNet_Quit();
	SDL_Quit();

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

// OpenSSL implementations

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

unsigned char* sumHash(unsigned char* h1, unsigned char * h2)
{
	// Concat both inputs
	uint32_t concat_hash_length = SHA256_DIGEST_LENGTH * 2;
	unsigned char* concat_hash = new unsigned char[concat_hash_length];

	memcpy_s(concat_hash, SHA256_DIGEST_LENGTH, h1, SHA256_DIGEST_LENGTH);
	concat_hash += SHA256_DIGEST_LENGTH;
	memcpy_s(concat_hash, SHA256_DIGEST_LENGTH, h2, SHA256_DIGEST_LENGTH);
	concat_hash -= SHA256_DIGEST_LENGTH;

	// Calculate hashes
	unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
	sha256(concat_hash, concat_hash_length, hash);

	return hash;
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

std::vector<Node*> Node::getVerifyChain(Tree tree)
{
	std::vector<Node*> chain;

	Node* parent = this;

	while (parent)
	{
		Node* parent_sibling = parent->getSibling(tree);
		if(parent_sibling)
			chain.push_back(parent_sibling);

		parent = parent->getParent(tree);
	}

	return chain;
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

NetworkBlock Node::toNetworkBlock()
{
	NetworkBlock nBlock;

	nBlock.id = this->id;
	std::memcpy(nBlock.hash, hash, 32);

	return nBlock;
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
	for (int i = 0; i < str_length; i++)
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

std::string hexStrToCharStr(std::string str)
{
	int str_size = str.size();
	std::string string;

	if (str_size & 1)
		return string;

	for (int i = 0; i < str_size; i += 2)
	{
		int8_t h1 = str.at(i);
		int8_t h2 = str.at(i + 1);
		char d1 = charToInt8(h1) * 16;
		char d2 = charToInt8(h2);
		char r = d1 + d2;

		string.push_back(r);
	}

	return string;
}

int8_t charToInt8(char c)
{
	if (c > 96 && c < 103)
		return (int8_t)(c - 87);
	else
		return (int8_t)(c - 48);
}

// Verifier

void selfProgramVerify(uint32_t tree_size)
{
	uint32_t input_id  = 1;
	while (input_id != 0)
	{
		// Ask for user input
		std::cout << "Enter a block number id to check if it belongs to the tree\n";
		std::cin >> input_id;

		// Search given block
		input_id = tradLabelToStandard(input_id, tree_size);
		Node* seeked_block = tree.at(input_id);

		// Check if it is a block
		if (!seeked_block->isBlock())
			continue;

		// Get verify chain
		std::vector<Node*> chain = seeked_block->getVerifyChain(tree);

		// Verify chain
		Node* root = tree.at(1);
		bool belong = verifyBlock(chain, seeked_block, root);

		// Inform with output

		if (belong)
			std::cout << "The block was verfied correctly\n\n";
		else
			std::cout << "The block was not verified correctly\n\n";
	}
}

bool verifyBlock(std::vector<Node*> chain, Node * block, Node * root)
{
	unsigned char* hash = block->hash;

	Node* sibling = nullptr;
	for (int i = 0; i < chain.size(); i++)
	{
		sibling = chain.at(i);
		if (sibling->id & 1)
			hash = sumHash(sibling->hash, hash);
		else
			hash = sumHash(hash, sibling->hash);
	}

	return !std::memcmp(hash, root->hash, SHA256_DIGEST_LENGTH);
}

bool verifyById(int32_t id)
{
	id = tradLabelToStandard(id, tree.size());
	Node* seeked_block = tree.at(id);

	// Check if it is a block
	if (!seeked_block->isBlock())
		return false;

	// Get verify chain
	std::vector<Node*> chain = seeked_block->getVerifyChain(tree);

	// Verify chain
	Node* root = tree.at(1);
	bool belong = verifyBlock(chain, seeked_block, root);

	// Inform with output
	if (belong)
		std::cout << "The block" << id << " was verfied correctly\n\n";
	else
		std::cout << "The block" << id << " was not verified correctly\n\n";
}

std::vector<Node*> getVerifyChainById(int32_t id)
{
	id = tradLabelToStandard(id, tree.size());
	Node* seeked_block = tree.at(id);
	std::vector<Node*> chain;

	// Check if it is a block
	if (!seeked_block->isBlock())
		return chain;

	// Get verify chain
	chain = seeked_block->getVerifyChain(tree);

	return chain;
}

// Networking

int initNetworkingLibs()
{
	if (SDL_Init(0) == -1)
	{
		std::cout << "SDL_Init: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}
	if (SDLNet_Init() == -1)
	{
		std::cout << "SDLNet_Init: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	return 0;
}

int initProverServer(UDPsocket sock)
{
	// Anounce
	std::cout << "Initializing network services\n";

	// Allocate a packet
	UDPpacket* packet = SDLNet_AllocPacket(sizeof(int32_t));

	// Try to recieve packets
	Uint8 status = 0;
	while (true)
	{
		status = SDLNet_UDP_Recv(sock, packet);

		// Check for recieved packets
		if (status)
		{
			// Get the id from the packet
			int32_t id = 0;
			std::memcpy(&id, packet->data, packet->len);

			// Check exit block
			if (id == -1)
			{
				std::cout << "Finishing packet recieved\n";
				SDLNet_FreePacket(packet);
				return 0;
			}

			// Get block chain
			std::vector<Node*> chain = getVerifyChainById(id);

			if (chain.empty())
			{
				std::cout << "Recieved invalid id/n";
				continue;
			}

			// Parse to binary data
			std::vector<NetworkBlock> nChain = nodeChainToNetworkChain(chain);
		}
		// Check for errors
		if (status == -1)
		{
			std::cout << "SDLNet_UDP_Recv: " << std::string(SDLNet_GetError()) << std::endl;
			return -1;
		}

		// Delay some time to prevent 100% CPU Usage
		SDL_Delay(200);
	}

	return 0;
}

int sendResponse(UDPsocket sock, IPaddress dest)
{
	// Bind
	Uint8 channel = SDLNet_UDP_Bind(sock, -1, &dest);

	if (channel == -1)
	{
		std::cout << "SDLNet_UDP_Bind: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	// Get

	return 0;
}

std::vector<NetworkBlock> nodeChainToNetworkChain(std::vector<Node*> chain)
{
	std::vector<NetworkBlock> bChain;

	for (auto const& node : chain)
	{
		NetworkBlock block = node->toNetworkBlock();
		bChain.push_back(block);
	}

	return bChain;
}