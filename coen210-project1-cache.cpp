// coen210-project1-cache.cpp : This file contains the 'main' function. Program execution begins and ends there.
/*
Brenda Wang
10/3/2021
COEN 210 Project 1
*/

#include "Header.h"

int main()
{
	//filenames
	std::string addressFile = "inst_addr_trace_hex_project_1.txt"; //contains address instructions in form 0x00000000
	std::string dataSizeFile = "inst_data_size_project_1.txt"; //data size in hex value (or divide 2 for byte values) of data referred to by address instruction
	std::string dataTraceHexFile = "inst_data_trace_hex_project_1.txt"; //what the data and size referred to by address should be, check against this value
	std::string memHexFile = "inst_mem_hex_16byte_wide.txt"; //16-byte (32 hex values) (128-bit) wide memory 

	//create arrays and import files into them
	std::vector<std::string> addressArray = importToArray(addressFile);
	std::vector<std::string> dataSizeArray = importToArray(dataSizeFile);
	std::vector<std::string>  dataTraceHexArray = importToArray(dataTraceHexFile);
	std::vector<std::string>  memHexArray = importToArray(memHexFile);


	//memory 16 bytes (128-bits) per line
	//address line bit partition ====> 19 TAG BITS | 9 INDEX BITS | 4 OFFSET BITS

	//cache line partition ====> 1 VALID BIT | 19 TAG BITS | 128 DATA BITS
	//cache line partition ====> 1 VALID BIT | 19 TAG BITS | 32 HEX DATA (NOT BITS)
	
	Cache cache1; //create cache object

	//current iteration values from files
	std::string instruction; //instruction address
	int size; //size array
	std::string trace; //trace array
	
	//extracted values from address instruction
	int addressTag; // decimal tag retrieved from instruction
	int addressLine, indexCache; //decimal values for line in memory and line in cache;
	int offset; //data offset

	//counts
	int instructionCount = 0, straddleCount = 0;

	//start 
	for (size_t iterator = 0; iterator < addressArray.size() ; iterator++) {

		instructionCount++;
		instruction = strHexTostrBin(addressArray[iterator], 8); // address is binary string
		addressLine = strBinToDecInt(instruction.substr(0, 28), 28); //calculate line in memory to go to

		size = std::stoi(dataSizeArray[iterator]); //size is integer representing # hex data
		offset = strBinToDecInt(instruction.substr(28, 4), 4); //how many hex data to offset from LSB before reading data from cache
		int pos = 31 - offset * 2; //calculate start index position [0,31] for the offset from LSB for 32 hex data subarray 

		trace = dataTraceHexArray[iterator]; //use to validate that data read for each instruction is correct

		int tag_index_arr[2];
		getTagAndIndex(instruction, tag_index_arr); //get the tag and index from instruction
		addressTag = tag_index_arr[0]; //integer value tag
		indexCache = tag_index_arr[1]; //integer value cache index

		bool straddlesCacheLine = (size <= pos+1) ? false : true; //check if 1 (false doesn't straddle) or 2 (true does straddle) reads needed

		//first read
		if (!cache1.cacheLookup(indexCache, addressTag)) { //check cache

			//if miss, go to memory and copy data to cache
			++cache1.missCount;
			cache1.retrieveFromMemory(memHexArray, addressLine, indexCache, addressTag);
		}
		else {
			++cache1.hitCount; //or hit, proceed to read
		}
		int read_size = (straddlesCacheLine) ? pos+1 : size; //if only partial first read, read from position to index 0
		cache1.readCacheLine(indexCache, read_size, pos, false); //read cache for data

		//optional second read 
		if (straddlesCacheLine) { 

			straddleCount++;

			bool lastlineofcache = (indexCache + 1 == 512) ? true : false; //when true & data straddles index 511 and 0 of cache, tag is also ++

			indexCache = (++indexCache) % 512; //increment index to next one (511->0)

			if (!cache1.cacheLookup(indexCache, addressTag)) { //check cache at next index and same tag (unless last line) 

				//if miss, go to memory and copy data to cache
				++cache1.missCount;
				if (lastlineofcache) {
					++addressTag; //when instruction address's index=111111111 (the last line), an increment will cause the tag to increment as well 
				}
				cache1.retrieveFromMemory(memHexArray, ++addressLine, indexCache, addressTag); //increase address line in memory to next one

			}
			else {
				++cache1.hitCount; //or hit, proceed to read 
			}

			read_size = size - (pos+1); //remaining amt data to read 
			cache1.readCacheLine(indexCache, read_size, 31, true); //read cache for data starting from LSB and prepend to data from first read
		}

		cache1.validateData(trace, iterator); //when finished reading, validate data with trace
 
	} //end loop

	//calculations
	int total_clock_cycles = 1 * cache1.hitCount + 15 * cache1.missCount;
	int total_accesses = cache1.hitCount + cache1.missCount;
	double hit_ratio = (double)cache1.hitCount / (total_accesses);
	double instructions_per_cycle = (double)instructionCount / total_clock_cycles;

	std::cout << "total instruction addresses = " << instructionCount << std::endl;
	std::cout << "hit count = " << cache1.hitCount << "\nmiss count = " << cache1.missCount << std::endl;
	std::cout << "total data accesses = " << total_accesses << std::endl;
	std::cout << "total straddles = " << straddleCount << std::endl;
	std::cout << "total clock cycles = " << total_clock_cycles << std::endl;
	std::cout << "hit ratio = " << hit_ratio << std::endl;
	std::cout << "instructions per cycle (IPC) = " << instructions_per_cycle << std::endl;

	return 0;
}
