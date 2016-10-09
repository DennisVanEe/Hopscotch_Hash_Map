#include <iostream>
#include <time.h>
#include <stdexcept>

#include "HashMap.hpp"

void displayHelp()
{
	std::cout << "construct [size]   Constructs a hashmap of size [size]. Only one instance may exist at a time." << std::endl;
	std::cout << "destruct           Destructs the current hashmap" << std::endl;
	std::cout << "set [key] [value]  Sets a pointer of value [value] at [key]" << std::endl;
	std::cout << "get [key]          Returns the value at [key]" << std::endl;
	std::cout << "remove [key]       Returns and removes a value at [key]" << std::endl;
	std::cout << "load               Returns the load of the hasmap" << std::endl;
	std::cout << "test [size]        Runs a test of random keys and values, constructs new map" << std::endl;
	std::cout << "exit               Exits the program" << std::endl;
}

// used for positioning index location for commands with more than one input
unsigned int getIndex;
unsigned int getNum(const std::string& val, size_t start)
{
	getIndex = start;
	std::string num;
	for (getIndex = start; val[getIndex] != ' ' && val[getIndex] != '\n' && getIndex < val.size(); getIndex++)
	{
		num += val[getIndex];
	}

	return std::stoi(num);
}

std::string getKey(const std::string& val, size_t start)
{
	std::string key;
	for (getIndex = start; val[getIndex] != ' ' && val[getIndex] != '\n' && getIndex < val.size(); getIndex++)
	{
		key += val[getIndex];
	}

	return std::move(key);
}

void shuffle(int *arr, size_t n)
{
	if (n > 1)
	{
		size_t i;
		//srand(time(NULL));
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = arr[j];
			arr[j] = arr[i];
			arr[i] = t;
		}
	}
}

char randomChar()
{
	std::string alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	return alphabet[rand() % alphabet.size()];
}

void test_hashMap(HashMap* map, uint32_t size)
{
	using Pair = std::pair<std::string, void*>;
	Pair* testArr = new Pair[size];

	for (uint32_t i = 0; i < size; i++)
	{
		int stringLength = 1 + rand() % 15;
		std::string key = "";
		for (int j = 0; j < stringLength; j++)
		{
			key += randomChar();
		}

		void* randData = reinterpret_cast<void*>(1 + rand() % 4294967);

		testArr[i] = std::make_pair(key, randData);
	}

	for (uint32_t i = 0; i < size; i++)
	{
		if (!map->set(testArr[i].first, testArr[i].second))
			std::cout << "Failed for key: " << testArr[i].first << std::endl;
	}

	std::cout << "Finished filling the map" << std::endl;

	for (uint32_t i = 0; i < size; i++)
	{
		if (map->get(testArr[i].first) != testArr[i].second)
		{
			uint32_t j = 0;
			for (; j < size; j++)
			{
				if (testArr[j].first == testArr[i].first && j != i)
				{
					break;
				}
			}

			if (j == size)
			{
				std::cout << "Failed retrieving the key at: " << testArr[i].first << " though it isn't a duplicate" << std::endl;
			}
		}
	}

	std::cout << "Finished retrieval test" << std::endl;
	std::cout << "Testing stage is done" << std::endl;
}

//
// Command Line
// A simple command line interface for testing the HashMap
int main()
{
	srand(time(nullptr));
	HashMap* map = nullptr;

	std::cout << "HashMap" << std::endl;
	std::cout << "-------" << std::endl << std::endl;
	std::cout << "type help to see commands" << std::endl;

	std::string inputBuffer;
	while (true)
	{
		std::cout << "> ";
		std::getline(std::cin, inputBuffer);

		if (inputBuffer.find("help") == 0)
		{
			displayHelp();
		}
		else if (inputBuffer.find("construct ") == 0)
		{
			if (map != nullptr)
				std::cout << "Instance of HashMap already exists, one instance at a time" << std::endl;
			else
			{
				unsigned int size;

				try
				{
					size = getNum(inputBuffer, 10);
				}
				catch (...)
				{
					std::cout << "not a valid number" << std::endl;
					continue;
				}

				try
				{
					map = new HashMap(size);
				}
				catch (const std::logic_error& e)
				{
					std::cout << e.what() << std::endl;
					continue;
				}

				std::cout << "HashMap of size " << size << " created" << std::endl;
			}
		}
		else if (inputBuffer.find("destruct") == 0)
		{
			if (map == nullptr)
				std::cout << "No instance of HashMap created" << std::endl;
			else
			{
				map->~HashMap();
				map = nullptr;
				std::cout << "HashMap was destructed" << std::endl;
			}
		}
		else if (inputBuffer.find("test ") == 0)
		{
			if (map != nullptr)
				std::cout << "Instance of HashMap already exists, one instance at a time" << std::endl;
			else
			{
				unsigned int size;

				try
				{
					size = getNum(inputBuffer, 5);
				}
				catch (...)
				{
					std::cout << "not a valid number" << std::endl;
					continue;
				}

				try
				{
					map = new HashMap(size);
				}
				catch (const std::logic_error& e)
				{
					std::cout << e.what() << std::endl;
					continue;
				}

				std::cout << "HashMap of size " << size << " created" << std::endl;
				std::cout << "Starting test..." << std::endl;
				test_hashMap(map, size);
			}
		}
		else if (inputBuffer.find("set ") == 0)
		{
			if (map == nullptr)
				std::cout << "No instance of HashMap created" << std::endl;
			else
			{
				std::string key = getKey(inputBuffer, 4);
				getIndex++;
				uintptr_t pointer;
				try
				{
					pointer = getNum(inputBuffer, getIndex);
				}
				catch (...)
				{
					std::cout << "invalid pointer value" << std::endl;
					continue;
				}

				if (map->set(key, reinterpret_cast<void*>(pointer)))
				{
					std::cout << "Succesfully set pointer value " << pointer << " for key " << key << std::endl;
				}
				else
				{
					std::cout << "Unsuccesfully set pointer value " << pointer << " for key " << key << std::endl;
				}
			}
		}
		else if (inputBuffer.find("get ") == 0)
		{
			if (map == nullptr)
				std::cout << "No instance of HashMap created" << std::endl;
			else
			{
				std::string key = getKey(inputBuffer, 4);
				void* returnVal = map->get(key);
				if (returnVal == nullptr)
				{
					std::cout << key << " does not exist in the map" << std::endl;
				}
				else
				{
					std::cout << "The value at " << key << " is " << reinterpret_cast<uintptr_t>(returnVal) << std::endl;
				}
			}
		}
		else if (inputBuffer.find("remove ") == 0)
		{
			if (map == nullptr)
				std::cout << "No instance of HashMap created" << std::endl;
			else
			{
				std::string key = getKey(inputBuffer, 7);
				void* returnVal = map->remove(key);
				if (returnVal == nullptr)
				{
					std::cout << key << " does not exist in the map" << std::endl;
				}
				else
				{
					std::cout << "The value at " << key << " is " << reinterpret_cast<uintptr_t>(returnVal) << " and was removed" << std::endl;
				}
			}
		}
		else if (inputBuffer.find("load") == 0)
		{
			if (map == nullptr)
				std::cout << "No instance of HashMap created" << std::endl;
			else
			{
				std::cout << "The current load is " << map->load() << std::endl;
			}
		}
		else if (inputBuffer.find("exit") == 0)
		{
			std::cout << "exiting..." << std::endl;
			return 0;
		}
		else
		{
			std::cout << "Invalid command, type help to see commnads" << std::endl;
		}
	}

	return 0;
}