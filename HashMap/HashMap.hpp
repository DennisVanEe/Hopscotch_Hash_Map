#pragma once

#include <string>
#include <stdint.h>

// If you want to manually set the hash value for testing specific scenarios
// #define DEBUG_HASH

// A simple function used to calculate the difference between unsigned types
template<typename T>
inline T UNSIGNED_DIFF(T a, T b)
{
	return a > b ? a - b : b - a;
}

// 
// HashMap
// The hash map is based on the idea that each bucket (index) of the map has a neighborhood
// that it belongs to. And that we can shift empty buckets closer in case of a collision
// to a neighborhood if the neighborhood were to be full so that it may still reside in the neighborhood.
// The advantage of neighborhoods is lookup: one must only search a set few buckets when comparing keys
// in case of a collision, unlike with linear or quadratic probing. 

// This idea is based on the HopScotch Hashing Map proposed by Herlihy, Shavit, and Tzafrir in a 2008 paper.
// However, this model maintains consistant neighborhood sizes (even near the ends of the array), centers
// the base of the neighborhood for more even spread, and uses linked lists to prevent resizing.

class HashMap
{
public:
	HashMap(unsigned int size);
	~HashMap();

	bool set(const std::string& key, void* value);
	void* get(const std::string& key)
	{
		uint32_t index = find(key);
		if (index == mSize)
		{
			return nullptr;
		}
		return mMap[index].data;
	}
	void* remove(const std::string& key)
	{
		uint32_t index = find(key);
		if (index == mSize)
		{
			return nullptr;
		}
		void* deletedData = mMap[index].data;
		mMap[index].data = nullptr;
		mItems--;
		return deletedData;
	}
	float load() const
	{
		return static_cast<float>(mItems) / mSize;
	}

	// for debug purposes:
#ifdef DEBUG_HASH
	void assignHash(uint32_t hash)
	{
		fakeHash = hash;
	}
#endif

private:

	// The neighborhood size
	const unsigned int mNeighborhoodSize = 4;

	// Info regarding the map's load
	unsigned int mSize;
	unsigned int mItems;

	// The definition for the individual buckets
	struct Bucket
	{
		void* data;
		std::string key;
		uint32_t baseBucket;

		// The bucket uses a doubly linked list so that it is easier
		// to update the previous bucket to a new bucket's position when it 
		// gets swapped.
		int nextOffset;
		int previousOffset; 

		Bucket(void* _data, const std::string& _key, uint32_t _baseBucket) :
			data(_data),
			key(_key),
			nextOffset(0),
			previousOffset(0),
			baseBucket(_baseBucket) {}
		Bucket() :
			data(nullptr),
			nextOffset(0),
			previousOffset(0) {}
	};

	// A pointer to the data itself
	Bucket* mMap;

	// Methods that aid in making the code easier to write:
	bool insertBucket(const std::string& key, void* value, uint32_t base, uint32_t index, int previousOffset = 0)
	{
		mMap[index] = Bucket(value, key, base);
		mMap[index].previousOffset = previousOffset;
		mItems++;
		return true;
	}
	bool inNeighborhood(uint32_t base, uint32_t index)
	{
		uint32_t unsignedDiff = UNSIGNED_DIFF(base, index);
		if (UNSIGNED_DIFF(base + 1, mSize) < mNeighborhoodSize / 2) // in the upper bounded region
		{
			return unsignedDiff <= mNeighborhoodSize - UNSIGNED_DIFF(base + 1, mSize);
		}
		else if (base < mNeighborhoodSize / 2)
		{
			return index <= mNeighborhoodSize;
		}
		else
		{
			return unsignedDiff <= mNeighborhoodSize / 2;
		}
	}
	uint32_t find(const std::string& key);
	uint32_t lowerBound(uint32_t index)
	{
		uint32_t diff = UNSIGNED_DIFF(index + 1, mSize);
		if (diff < mNeighborhoodSize / 2)
		{
			return index - (mNeighborhoodSize - diff);
		}
		else if (index < mNeighborhoodSize / 2)
		{
			return 0;
		}
		else
		{
			return index - mNeighborhoodSize / 2;
		}
	}

	// used by the insertion method to replace the hash
#ifdef DEBUG_HASH
	uint32_t fakeHash = 0;
#endif
};
