#include "BitCommit.h"

// Bit commitment protocol
// 1- Alice generates a random bit string, r. Then she sends it to Bob
// 2- Bob generates b0. (He flips a coin)
// 3- Bob generates a random bint string s.
// 4- Bob calculates c, by using s, r and b0. Then he sends it to Alice.
// 5- Alice generates b1 (She bets) and sends that value to Bob
// 6- Bob sends b0 and s to Alice
// 7- Alice verifies the commitment sent by Bob by calculating c with the values she recieved during the protocol
// 8- Alice checks if she has won the bet

// Requirements for the protocol
// The protocol can be proven to be correct, in that no one of the parties can drive the result to its own interest.
// Hiding: c should not reveal information about b0
// Binding: c can only be open as some b, but not as (not b)
// Seed and Result Length: R length must be at least three times bigger than S

// Global variables

// Seed generator
// Pseudo Random Generator
std::mt19937 dre;

// Random bitstring sizes
const int32_t R_SIZE = 48;
const int32_t S_SIZE = 16;

// Debug Mode
bool debug = true;

// Main
int main(int argc, char** args)
{
	// Initializing random generator with hardware's entropy
	initRandomGenerator();

	while (true)
	{
		// Create parties
		Guesser alice = Guesser();
		CoinFlipper bob = CoinFlipper();

		// 1 - Alice generates r and sends it to Bob
		log("1 - Alice generates r\n");
		alice.r = alice.generate_r();
		bob.r = alice.r;
		log(bitstringToString(alice.r) + "\n\n");

		// 2 - Bob generates b0 (He flips a coin)
		bob.b0 = bob.flipCoin();
		log("2 - Bob flips a coin, b0 = " + bitToStr(bob.b0) + "\n\n");

		// 3 - Bob generates a random bitstring s
		log("3 - Bob generates s\n");
		bob.s = bob.generate_s();
		log(bitstringToString(bob.s) + "\n\n");

		// 4 - Bob calculates c, by using s, r and b0. Then he sends it to Alice.
		log("4 - Bob calculates commitment c\n");
		bob.commitment = bob.calculateCommitment();
		log(bitstringToString(bob.commitment) + "\n\n");
		
		alice.commitment = bob.commitment;

		// 5- Alice generates b1 (She bets) and sends that value to Bob
		alice.b1 = alice.bet();
		bob.b1 = alice.b1;
		log("5 - Alice generates b1 = " + bitToStr(alice.b1) + "\n\n");

		// 6- Bob sends b0 and s to Alice
		log("6 - Bob sends b0 and s to Alice\n\n");
		alice.b0 = bob.b0;
		alice.s = bob.s;

		// Uncomment the following line to provoke a failed verification
		// alice.b0 = !bob.b0;

		// 7- Alice verifies the commitment sent by Bob by calculating c with the values she recieved during the protocol
		log("7 - Alice verifies the commitment sent by Bob by calculating c with the values she recieved during the protocol\n\n");
		bool result = alice.verify();
		printVerificationMessage(result);
		log("\n\n");
		
		// Only check winner in case commitment verification was correct
		if (result)
		{
			// 8 - Alice checks if she has won the bet
			bool won = alice.b0 == alice.b1;
			printResultMessage(won);
		}
		else
			log("Winner check was aborted due to a failed verification");

		// Wait for input before exiting the program
		std::cout << "\n\nPress ENTER to play again, press Ctrl-c to exit the program";
		getchar();

		std::cout << "\n\n\n";
	}
}

// Random

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

// Bit string operations
bitstring generateRandomBitString(int32_t length, int32_t seed)
{
	bitstring bitstr;
	bool reseed = seed != 0;

	// If seed is not zero, we set the value
	if (reseed)
		dre.seed(seed);

	for (int32_t i = 0; i < length; i++)
	{
		bool bit = false;

		// If the random engine was seeded, the operation needs to be deterministic
		if (reseed)
			bit = getRandomBit();
		// Secure random bit otherwise
		else
			bit = getSecureRandomBit();

		bitstr.push_back(bit);
	}

	return bitstr;
}

bit getRandomBit()
{
	std::bernoulli_distribution bd;
	return bd(dre);
}

bit getSecureRandomBit()
{
	// OpenSSL hardware random generation
	// Generate 8 bit number
	unsigned char* buffer = new unsigned char[1];
	int rc = RAND_pseudo_bytes(buffer, 1);

	if (rc != 1)
		log("Error while generating a random key");

	char number = buffer[0];

	// We return whether the number is odd or not
	// The same process as returning just the first bit of the given number
	return number & 1;
}

bitstring xor_bitstring(bitstring c, bitstring r)
{
	bitstring total;

	if (c.size() != r.size())
	{
		log("Warning: c and r are not the same length\n");
		return total;
	}
	for (int i = 0; i < c.size(); i++)
	{
		if (i < r.size())
		{
			bit bit = c.at(i) ^ r.at(i);
			total.push_back(bit);
		}
	}

	return total;
}

// Debug

void log(std::string str)
{
	if(debug)
		std::cout << str.c_str();
}

void printResultMessage(bool result)
{
	if (result)
		std::cout << "Alice has guessed the coin correctly and wins the bet\n";
	else
		std::cout << "Alice has not guessed the coin correctly and Bob wins the bet\n";
}

void printVerificationMessage(bool result)
{
	if (result)
		log("The commitment that Bob sent was verified correctly");
	else
		log("The commitment that Bob sent was NOT verified correctly");
}

// Utilities

std::string bitToStr(bit bit)
{
	return bit ? std::string("1") : std::string("0");
}

long bitstringToInteger(bitstring bitstr)
{
	long number = 0;

	for (int i = 0; i < bitstr.size(); i++)
	{
		// Left shifts the stored bit i times and the it's added to the number
		number += (bitstr.at(i) << i);
	}

	return number;
}

std::string bitstringToString(bitstring bitstr)
{
	std::string str;

	for (int i = 0; i < bitstr.size(); i++)
	{
		// Left shifts the stored bit i times and the it's added to the number
		str += bitToStr(bitstr.at(i));
	}

	return  str;
}

// Program dependant

// Participant method definitions

bitstring Participant::calculateCommitment(bitstring s, bitstring r, bit b0)
{
	// Any of the bitstrings has not been generated yet
	if (s.size() == 0 || r.size() == 0)
	{
		log("s or r size was invalid");
		return bitstring();
	}

	bitstring commitment;

	commitment = generateRandomBitString(R_SIZE, bitstringToInteger(s));

	if (b0)
		commitment = xor_bitstring(commitment, r);

	return commitment;
}

bitstring Participant::calculateCommitment()
{
	return calculateCommitment(s, r, b0);
}

// Guesser method definitions

bitstring Guesser::generate_r()
{
	return generateRandomBitString(R_SIZE);
}

bit Guesser::bet()
{
	return getRandomBit();
}

bool Guesser::verify(bitstring s, bitstring r, bit b0, bitstring recv_commitment)
{
	bitstring calculated_commitment = calculateCommitment(s, r, b0);

	return calculated_commitment == recv_commitment;
}

bool Guesser::verify()
{
	return verify(s, r, b0, commitment);
}

// Coin Flipper method definitions

bitstring CoinFlipper::generate_s()
{
	return generateRandomBitString(S_SIZE);
}

bit CoinFlipper::flipCoin()
{
	return getRandomBit();
}