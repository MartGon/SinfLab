#include "Verifier.h"

int main(int arg, char* argv[])
{

	if (arg != 4)
	{
		std::cout << "Expected a origi, destination port and an block id argument  \n";
		return -1;
	}

	// Init Networking libs
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

	// Ports
	uint16_t src_port = std::stoi(argv[1]);
	Uint16 dest_port = std::stoi(argv[2]);

	// Data to be sent
	int32_t data = std::stoi(argv[3]);

	// Get remote host ipaddress
	IPaddress localhost;
	int result = SDLNet_ResolveHost(&localhost, "localhost", dest_port);

	if (result == -1)
	{
		std::cout << "SDLNet_ResolveHost " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	// Creating socket
	UDPsocket udp_socket;
	udp_socket = SDLNet_UDP_Open(src_port);

	if (!udp_socket)
	{
		std::cout << "SDLNet_UDP_Open: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	// Binding socket
	Uint8 channel = SDLNet_UDP_Bind(udp_socket, -1, &localhost);
	if (channel == -1)
	{
		std::cout << "SDLNet_UDP_Bind: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	// Create packet
	UDPpacket *packet;
	packet = SDLNet_AllocPacket(sizeof(int32_t));

	if (!packet)
	{
		std::cout << "SDLNet_AllocPacket: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}
		
	// Fill packet data
	std::memcpy(packet->data, &data, sizeof(int32_t));
	packet->len = sizeof(int32_t);

	// Send packet
	result = SDLNet_UDP_Send(udp_socket, channel, packet);

	if (!result)
	{
		std::cout << "SDLNet_UDP_Send: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	// Check for close serve packet
	if (data == -1)
	{
		std::cout << "Close server packet sent\n";
		return 0;
	}

	// Create space for recieving packets
	SDLNet_FreePacket(packet);
	packet = SDLNet_AllocPacket(sizeof(NetworkBlock));

	// Recieving loot
	std::vector<NetworkBlock> chain;
	int8_t timeout = 5;
	while (true)
	{
		Uint8 recv_message = SDLNet_UDP_Recv(udp_socket, packet);

		if (recv_message == -1)
		{
			std::cout << "SDLNet_UDP_Recv: " << std::string(SDLNet_GetError()) << std::endl;
			return -1;
		}
		else if (!recv_message)
		{
			std::cout << "Could not recieve any packet\n";
			SDL_Delay(1000);
			timeout--;

			if (!timeout)
			{
				std::cout << "Connection timed out\n";
				return -1;
			}

			continue;
		}

		// Read block from packet 
		NetworkBlock recv_block;

		// Get data from packet
		std::memcpy(&recv_block, packet->data, packet->len);
		std::cout << "Dat id recv is " << std::to_string(recv_block.id) << std::endl;
		std::cout << "Dat hash recv is " << ucharToString(recv_block.hash, 32) << std::endl;

		// Check for erros
		if (recv_block.id == -1)
		{
			std::cout << "Sent block was not valid\n";
			return -1;
		}

		// Add to the list
		chain.push_back(recv_block);

		// Break when we get root block
		if (recv_block.id == 1)
			break;

		// Reset timeout
		timeout = 5;
	}

	// Verify data
	bool integ = verifyBlock(chain, chain.front(), chain.back());

	if (integ)
		std::cout << "Block was verified correctly\n";
	else
		std::cout << "Block was not verified correctly\n";

	// Close udp socket
	SDLNet_UDP_Close(udp_socket);

	// Clean packet 
	SDLNet_FreePacket(packet);

	// Shutdown SDL_net
	SDLNet_Quit();

	// Shutdown SDL
	SDL_Quit();

	return 0;
}

std::string ucharToString(unsigned char * str, int32_t str_length)
{
	std::stringstream ss;
	for (int i = 0; i < str_length; i++)
	{
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)str[i];
	}
	return ss.str();
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

	std::memcpy(concat_hash, h1, SHA256_DIGEST_LENGTH);
	concat_hash += SHA256_DIGEST_LENGTH;
	std::memcpy(concat_hash, h2, SHA256_DIGEST_LENGTH);
	concat_hash -= SHA256_DIGEST_LENGTH;

	// Calculate hashes
	unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
	sha256(concat_hash, concat_hash_length, hash);

	return hash;
}

// Verify

bool verifyBlock(std::vector<NetworkBlock> chain, NetworkBlock block, NetworkBlock root)
{
	unsigned char* hash = block.hash;

	NetworkBlock sibling;
	// Avoid self and root
	for (int i = 1; i < chain.size() - 1; i++)
	{
		sibling = chain.at(i);
		if (sibling.id & 1)
			hash = sumHash(sibling.hash, hash);
		else
			hash = sumHash(hash, sibling.hash);
	}

	return !std::memcmp(hash, root.hash, SHA256_DIGEST_LENGTH);
}