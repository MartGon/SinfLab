#include "Verifier.h"

int main(int arg, char* argv[])
{

	if (arg != 4)
	{
		std::cout << "Expected a origi, destination port and an id argument  \n";
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
	int8_t data = std::stoi(argv[3]);

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
	packet = SDLNet_AllocPacket(sizeof(Uint8));

	if (!packet)
	{
		std::cout << "SDLNet_AllocPacket: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}
		
	// Fill packet data
	*packet->data = data;
	packet->len = sizeof(Uint8);

	// Send packet
	result = SDLNet_UDP_Send(udp_socket, channel, packet);

	if (!result)
	{
		std::cout << "SDLNet_UDP_Send: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}

	Uint8 recv_message = SDLNet_UDP_Recv(udp_socket, packet);

	if (recv_message == -1)
	{
		std::cout << "SDLNet_UDP_Recv: " << std::string(SDLNet_GetError()) << std::endl;
		return -1;
	}
	else if (!recv_message)
	{
		std::cout << "Could not recieve any packet\n";
	}

	std::cout << "Dat recv is " << std::to_string(*packet->data) << std::endl;

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