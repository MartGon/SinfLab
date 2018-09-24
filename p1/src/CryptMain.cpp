#include "CryptMain.h"
#include <fstream>
#include <array>
#include <stdio.h>

std::map<int, std::vector<char>> global_count;

int main(int argc, char** args)
{
    std::vector<std::string> hex_msgs = getMsgsFromTextFile("file.txt");
    std::vector<std::string> char_msgs = parseHexMsgsToCharMsgs(hex_msgs);
    std::map<int, std::vector<char>> key_possibilites;
    std::string final_key = fill_string(' ', 1024);
    std::string final_key2;

    std::string a;
    std::string b;
    std::string total;
    std::vector<int> index;

    // Getting the key

    for (int i = 0; i < char_msgs.size(); i++)
    {
        a = char_msgs[i];
        for (int j = 0; j < char_msgs.size(); j++)
        {
            b = char_msgs[j];

            total = replaceNonLetters(xor_str(a, b), ' ');
            index = getIndexOfNonChr(total, ' ');

            getPossibleKeyValues(index, total, a, b, key_possibilites, final_key);
        }
    }

    for (auto it = global_count.begin(); it != global_count.end(); it++)
    {
        int key = it->first;
        final_key2.push_back(getMostFrequentValueFromVector(it->second));
    }

    std::cout << "\n\n" << final_key << "\n\n";

    // Printing plain text messages

    std::string plain_text;
    for (int i = 0; i < char_msgs.size(); i++)
    {
        plain_text = xor_str(final_key2, char_msgs[i]);

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

	if (str_size % 2)
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

void getPossibleKeyValues(std::vector<int> letter_index, std::string m_sum, std::string c1, std::string c2, std::map<int, std::vector<char>>& key_possiblities, std::string& final_key)
{
    std::map<int, std::vector<char>>& result = key_possiblities;

    char p1;
    char p2;

    for (int i = 0; i < letter_index.size(); i++)
    {
        int key_index = letter_index.at(i);

        //char m_char = m_sum.at(key_index);
        char m_char = ' ';
        p1 = c1.at(key_index) ^ m_char;
        p2 = c2.at(key_index) ^ m_char;

        if (result.find(key_index) == result.end())
        {
            result.insert_or_assign(key_index, std::vector<char>());
            result.at(key_index).push_back(p1);
            result.at(key_index).push_back(p2);
        }
        else
        {
            // If it is not set yet
            if (final_key.at(key_index) == ' ')
            {
                if (p1 == result.at(key_index).at(0) && p2 != result.at(key_index).at(1))
                {
                    final_key.at(key_index) = p1;
                }
                else if (p1 != result.at(key_index).at(0) && p2 == result.at(key_index).at(1))
                {
                    final_key.at(key_index) = p2;
                }
                else
                {
                    std::cout << "";
                }
            }
        }

        // Most frequent value measures
        if (global_count.find(key_index) == global_count.end())
        {
            global_count.insert_or_assign(key_index, std::vector<char>());
        }
        else
        {
            global_count.at(key_index).push_back(p1);
            global_count.at(key_index).push_back(p2);
        }
    }

    return;
}
