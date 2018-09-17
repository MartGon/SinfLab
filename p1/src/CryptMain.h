#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <algorithm>

// string operations

std::string fill_string(const char chr, int size);

std::string get_shorter(std::string a, std::string b);

std::vector<int> getIndexOfNonChr(std::string str, char chr);

std::string hexStrToCharStr(std::string str);

std::string xor_str(std::string a, std::string b);

// char operations

int8_t charToInt8(char c);

// vector

template <typename T>
void printVector(std::vector<T> vector)
{
    std::cout << "\n";
    for (T value : vector)
        std::cout << value << " ";
}

char getMostFrequentValueFromVector(std::vector<char> vector);

// Program dependant operations

std::vector<std::string> getMsgsFromTextFile(std::string filename);

std::vector<std::string> parseHexMsgsToCharMsgs(std::vector<std::string> hex_msgs);

std::string replaceNonLetters(std::string str, char chr);

std::string filter_string(const std::string& string, char max, char min = -1, char chr = ' ');

void getPossibleKeyValues(std::vector<int> letter_index, std::string m_sum, std::string c1, std::string c2, std::map<int, std::vector<char>>& key_possiblities, std::string& final_key);
