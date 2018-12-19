// C++ Standard Libs
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstring>

// OpenSSL libs
#include "openssl/sha.h"
#include <openssl/err.h>

// Network libs
#ifdef _WIN32
	#include <SDL.h>
	#include <SDL_net.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_net.h>
#endif

struct NetworkBlock
{
public:
	int32_t id;
	unsigned char hash[32];
};

// Readability

std::string ucharToString(unsigned char * str, int32_t str_length);

// OpenSSL implementations

void sha256(unsigned char *input, int32_t input_length, unsigned char* output);

unsigned char* sumHash(unsigned char* h1, unsigned char * h2);

// Verify

bool verifyBlock(std::vector<NetworkBlock> chain, NetworkBlock block, NetworkBlock root);