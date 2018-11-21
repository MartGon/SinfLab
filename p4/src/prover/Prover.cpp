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

	// Open file
	char* filename = argv[1];
	std::fstream myfile;
	myfile = std::fstream(filename, std::ios::in | std::ios::binary);

	// Read file
	bool eof = false;
	while (!eof)
	{
		// Get block
		BlockChainBlock block;
		block = getNextBlock(myfile, eof);
	}
	
	// Ask for user input
	std::cout << "Press enter to close the program\n";

	// Close file
	myfile.close();

	// Wait for user input
	std::cin.get();

	return 0;
}

BlockChainBlock getNextBlock(std::fstream& blockchain_file, bool &eof)
{
	int32_t index = blockchain_file.tellg();

	// Read Block
	BlockChainBlock block;

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
	block.magic_number = magic_number;
	readFromFile(blockchain_file, block.size);

	// Read header
	BlockChainHeader header;

	readFromFile(blockchain_file, header.version);
	readFromFile(blockchain_file, header.hashPrevBlock);
	readFromFile(blockchain_file, header.hashMerkleTree);
	readFromFile(blockchain_file, header.time);
	readFromFile(blockchain_file, header.bits);
	readFromFile(blockchain_file, header.nonce);

	block.header = header;

	// Read payload
	int32_t read_bytes = (int)blockchain_file.tellg() - index;
	int32_t left_bytes = block.size - read_bytes;
	block.payload = new unsigned char[left_bytes];
	blockchain_file.read((char*)block.payload, left_bytes);

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
