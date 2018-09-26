#include <vector>
#include <iostream>
#include <random>
#include <stdio.h>

// Tricks to make the code easier to read

using bitstring = std::vector<bool>;
using bit = bool;

// Bit string operations

bitstring generateRandomBitString(int32_t length, int32_t seed = 0);

bit getRandomBit();

long bitstringToInteger(bitstring bitstr);

// Debug

void log(std::string str);

void printResultMessage(bool result);

// Utilities

std::string bitToStr(bit bit);

// Program dependant

class Participant
{
public:

	// Random bit strings
	bitstring r;
	bitstring s;

	// Commitment
	bitstring commitment;
	
	// Coin values
	bit b0; // Flip
	bit b1; // Bet

	// Methods
	bitstring calculateCommitment(bitstring s, bitstring r, bit b0);
	bitstring calculateCommitment();
};

// Alice's role
class Guesser : public Participant
{
public:
	// Methods
	bitstring generate_r();
	bit bet();
	bool verify(bitstring s, bitstring r, bit b0, bitstring recv_commitment);
	bool verify();
};

// Bob's role
class CoinFlipper : public Participant
{
public:
	// Methods
	bitstring generate_s();
	bit flipCoin();
};
