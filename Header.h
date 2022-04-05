#pragma once

/*
Brenda Wang
10/3/2021
COEN 210 Project 1
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>

class Cache {
public:
	//cache line partition ====> 1 VALID BIT | 19 TAG BITS | 32 HEX DATA (NOT BITS)
	//initialize empty vector with 512 entries each with 52-char string
	std::vector<std::string> cache = std::vector<std::string>(512, "0000000000000000000000000000000000000000000000000000");
	
	int hitCount = 0;
	int missCount = 0;

	std::string data = "";

	//cache functions
	void readCacheLine(int, int, int, bool);
	bool cacheLookup(int, int);
	void retrieveFromMemory(std::vector<std::string>, int, int, int);
	void validateData(std::string, size_t);
};

//prototypes
std::vector<std::string> importToArray(std::string);

std::string strHexTostrBin(std::string, int);
int strBinToDecInt(std::string, int);
std::string decIntTostrBin(int, int);

void getTagAndIndex(std::string, int[]);


std::vector<std::string> importToArray(std::string filename) { //open the filename as a file object and import the data to an array
	std::ifstream file;
	file.open(filename);

	std::vector<std::string> vect;
	std::string line;

	while (std::getline(file, line)) {
		vect.push_back(line);
	}

	file.close();

	return vect;
}

std::string strHexTostrBin(std::string hexStr, int digits) { //convert a hex string to binary string

	std::string strBin = "";
	char hex;
	std::string add;

	for (int i = 0; i < digits; ++i) {

		hex = hexStr[i];

		switch (hex) {
		case '0': add = "0000"; break;
		case '1': add = "0001"; break;
		case '2': add = "0010"; break;
		case '3': add = "0011"; break;
		case '4': add = "0100"; break;
		case '5': add = "0101"; break;
		case '6': add = "0110"; break;
		case '7': add = "0111"; break;
		case '8': add = "1000"; break;
		case '9': add = "1001"; break;
		case 'a': add = "1010"; break;
		case 'b': add = "1011"; break;
		case 'c': add = "1100"; break;
		case 'd': add = "1101"; break;
		case 'e': add = "1110"; break;
		case 'f': add = "1111"; break;
		}

		strBin = strBin + add;
	}

	return strBin;
}

int strBinToDecInt(std::string binStr, int size) { //convert a binary string to a decimal interger

	int dec = 0;
	int twoExp = 1;
	std::string current;
	for (int i = size - 1; i >= 0; --i) {
		current = binStr[i];
		dec += std::stoi(current) * twoExp;
		twoExp *= 2;
	}
	return dec;
}

std::string decIntTostrBin(int decimal, int size) {	//convert a decimal integer to a binary string

	std::string binary = "";
	int i;
	for (i = 0; i < size; ++i) {
		binary.push_back('0');
	}
	char current;

	while (decimal > 0) {
		current = (decimal % 2) + '0';
		decimal /= 2;
		binary[--i] = current;
	}

	return binary;
}

void getTagAndIndex(std::string address, int a[]) { //return the tag and index encoded in an binary address instruction

	//address is form  ttttttttttttttttttt|iiiiiiiii|oooo

	std::string index_b = address.substr(19, 9); //extract index bits
	std::string tag_b = address.substr(0, 19); //extract tag bits

	a[0] = strBinToDecInt(tag_b, 19); //return a decimal tag
	a[1] = strBinToDecInt(index_b, 9); //return a decimal index
}



void Cache::readCacheLine(int index, int size, int offset_pos, bool multiline) {

	//given the size of data to read and the offset, read the data from cache at indicated index
	
	std::string temp = "";

	for (int i = 0; i < size / 2; ++i) { // half of size because reading 2 hex at a time
		temp = cache[index].substr(20 + offset_pos - 1, 2) + temp; // add 20 to account for valid+tag bits; reading 2 hex at a time right to left
		offset_pos -= 2;
	}
	
	//multiline is true means data read is the second half that spilled over to next cache block
	//and needs to be prepended (bc data read LSB->MSB) to data from first read 
	data = (!multiline) ? temp : temp + data;
}

bool Cache::cacheLookup(int index, int tag) { //check if the tag from address currently matches the tag at indicated index in cache or if invalid

	std::string block = cache[index]; //currently looking at

	int tagInCache = strBinToDecInt(block.substr(1, 19), 19); //tag starts at position 1

	if (block[0] == '0' || tagInCache != tag) //valid bit is 0 or tag doesn't match
		return false;
	else
		return true;
}

void Cache::retrieveFromMemory(std::vector<std::string> memory, int addressLine, int index, int tag) {
	//given the addressline in memory, copy data bits and tag bits and set valid bit of cache array

	std::string memory_data = memory[addressLine]; //get entire line in memory as a hex string

	//copy that line to the cache data section (as hex)
	int c_bit = 20; //initial position to add to cache array
	for (auto hex : memory_data) {
		cache[index][c_bit++] = hex;
	}

	cache[index][0] = '1'; //set valid bit in cache

	int i = 1;
	std::string b_tag = decIntTostrBin(tag, 19);
	for (auto bit : b_tag) {
		cache[index][i++] = bit; //set tag bits in cache
	}
}

void Cache::validateData(std::string trace, size_t iterator) {
	//confirm that data read from cache for an instruction matches the trace file; print an error to screen if incorrect

	int i = 0;
	char c;
	while (data[i]) {
		c = data[i];
		data[i] = std::toupper(c); //data's hex values -> uppercase
		++i;
	}
	i = 0;
	while (trace[i]) {
		c = trace[i];
		trace[i] = std::toupper(c); //trace's hex values -> uppercase
		++i;
	}
	if (data.compare(trace) != 0)
		std::cout << "Address " << iterator << " - incorrect" << " data= " << data << " trace= " << trace << std::endl;
}
