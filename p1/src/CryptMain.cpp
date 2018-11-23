#include "CryptMain.h"
#include <fstream>
#include <array>
#include <stdio.h>

int main(int argc, char** args)
{
	// First, we get the hex strings from the text file
    std::vector<std::string> hex_msgs = getMsgsFromTextFile("file.txt");

	// Then we parse those hex strings to char strings. This will allows us to use them more easily
    std::vector<std::string> char_msgs = parseHexMsgsToCharMsgs(hex_msgs);

	// Now we define the map that will hold the list of all the possible values of a given index of the key
    std::map<int, std::vector<char>> key_possibilites;

	// We'll store the final key here
	std::string final_key;

	// Variables to use during the loop
    std::string c1;
    std::string c2;
    std::string total;
    std::vector<int> index;

    // Loop for getting the possible values of the key for each index/position
	// The process of acquiring the key is made by following the next propierties/observations
	// 1- The most common charactes are going to be letters ([A-z]) and the character space ' '
	// 2- If we xor two ciphertexts together c1, c2 we get the sum of its plain texts; c1 + c2 = m1 + m2. Removing the key from the equation
	// 3- Because of the propierties of the ASCII table, it is impossible to form a letter ([A-z]) by xoring two letters ([A-z]). That's because the 64 bit needs to be always one.
	// 4- If we xor a letter with a space character, we get its lower or upper case counterpart
	// 4- Then, if we xor two ciphertexts and find a letter in the result string, we know that in one of the two plaintexts (m1 or m2) there is a character space in that position.
	// 5- With that, we reduce the number of possibilites of the value of that index of the key to two, p1 = space xor c1[index] or p2 = space xor c2[index]
	// 6- By repeating this process with each pair of ciphertext we can form a key aproximation by getting the most common value for each index of the key.
	// 7- Because of propierty 1 there we'll be some mistakes, but the decrypted text should be readable.
    for (int i = 0; i < char_msgs.size(); i++)
    {
        c1 = char_msgs[i];
        for (int j = 0; j < char_msgs.size(); j++)
        {
            c2 = char_msgs[j];

            total = replaceNonLetters(xor_str(c1, c2), ' ');
            index = getIndexOfNonChr(total, ' ');

            getPossibleKeyValues(index, total, c1, c2, key_possibilites);
        }
    }

	// Form the final key by getting the most common value for each index of the key
    for (auto it = key_possibilites.begin(); it != key_possibilites.end(); it++)
    {
        int key = it->first;
        final_key.push_back(getMostFrequentValueFromVector(it->second));
    }

    std::cout << "\n\n" << final_key << "\n\n";

    // Decrypting and printing plain text messages

    std::string plain_text;
    for (int i = 0; i < char_msgs.size(); i++)
    {
        plain_text = xor_str(final_key, char_msgs[i]);

        std::cout << "Message: " << (i + 1) << "\n" << plain_text << "\n\n";
    }

    getchar();

    return 0;
}

// string operations

std::string fill_string(char chr, int size)
{
	std::string str;

	for (int i = 0; i < size; i++)
		str.push_back(chr);

	return str;
}

std::string get_shorter(std::string a, std::string b)
{
	if (a.size() < b.size())
		return a;
	else
		return b;
}

std::vector<int> getIndexOfNonChr(std::string str, char chr)
{
	std::vector<int> index;

	for (int i = 0; i < str.size(); i++)
	{
		if (str.at(i) != chr)
			index.push_back(i);
	}

	return index;
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

std::string xor_str(std::string a, std::string b)
{
	std::string str;
	int iterations = get_shorter(a, b).size();

	for (int i = 0; i < iterations; i++)
	{
		char sum = a.at(i) ^ b.at(i);
		str.push_back(sum);
	}

	return str;
}

// char operations

int8_t charToInt8(char c)
{
	if (c > 96 && c < 103)
		return (int8_t)(c - 87);
	else
		return (int8_t)(c - 48);
}

// vector

char getMostFrequentValueFromVector(std::vector<char> vector)
{
	std::map<char, int> val_count;

	for (int i = 0; i < vector.size(); i++)
	{
		char index = vector.at(i);
		if (val_count.find(index) == val_count.end())
		{
			val_count.insert_or_assign(index, 1);
		}
		else
			val_count.at(index)++;
	}

	// Get max key which its value is the biggest on the map
	auto max = std::max_element(val_count.begin(), val_count.end(),
		[](const decltype(val_count)::value_type p1, const decltype(val_count)::value_type p2) {
		return p1.second < p2.second;
	});

	return max->first;
}

// Program dependant operations

std::vector<std::string> getMsgsFromTextFile(std::string filename)
{
    std::ifstream infile(filename);
    std::vector<std::string> msgs;

    if (infile.fail())
        return msgs;

   // std::cout << "Reading\n\n";

    std::string line;
    while (std::getline(infile, line))
    {
      //  std::cout << line << "\n";

        if (line.empty())
            continue;

        msgs.push_back(line);
    }

    return msgs;
}

std::vector<std::string> parseHexMsgsToCharMsgs(std::vector<std::string> hex_msgs)
{
    std::vector<std::string> char_msgs;

   // std::cout << "\n\n" << "Parsing";
    for (int i = 0; i < hex_msgs.size(); i++)
    {
        std::string char_msg = hexStrToCharStr(hex_msgs[i]);
        // std::cout << "\n\n" << char_msg << "\n";

        char_msgs.push_back(char_msg); 
    }

    return char_msgs;
}

std::string replaceNonLetters(std::string str, char chr)
{
	std::string total = str;

	total = filter_string(total, 65, -1, chr);
	total = filter_string(total, 97, 91, chr);
	total = filter_string(total, 127, 122, chr);

	return total;
}

std::string filter_string(const std::string& string, char max, char min, char chr)
{
    std::string str = string;

    // check valid values
    if (max < min)
        return str;
    else if (min > max)
        return str;

    for (int i = 0; i < str.size(); i++)
    {
        char c = (str.at(i));
        if (c < max && c > min)
            str.at(i) = chr;
    }

    return str;
}

void getPossibleKeyValues(std::vector<int> letter_index, std::string m_sum, std::string c1, std::string c2, std::map<int, std::vector<char>>& key_possiblities)
{
    std::map<int, std::vector<char>>& result = key_possiblities;

    char p1;
    char p2;

    for (int i = 0; i < letter_index.size(); i++)
    {
        int key_index = letter_index.at(i);

        char m_char = ' ';
        p1 = c1.at(key_index) ^ m_char;
        p2 = c2.at(key_index) ^ m_char;

        // Most frequent value measures
        if (result.find(key_index) == result.end())
        {
            result.insert_or_assign(key_index, std::vector<char>());
        }
        else
        {
            result.at(key_index).push_back(p1);
            result.at(key_index).push_back(p2);
        }
    }

    return;
}
