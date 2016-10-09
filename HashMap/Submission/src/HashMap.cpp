#include "Hash.hpp"
#include "HashMap.hpp"

#include <iostream>
#include <stdexcept>

// The neighborhood size depends on the size of the array itself.
// The paper points out that a max of 64 for the neighborhood size
// is optimal, and so, larger map sizes would not benefit otherwise.
// The values chosen have not been completely fine-tuned.
unsigned int chooseNeighborhoodSize(unsigned int _size)
{
	if (_size <= 2)
	{
		throw std::logic_error("The size of the map is too small for proper behaviour");
	}
	if (_size <= 4)
	{
		return _size - 1;
	}
	else if (_size <= 32)
	{
		return 4;
	}
	else if (_size <= 64)
	{
		return 8;
	}
	else if (_size <= 128)
	{
		return 16;
	}
	else if (_size <= 512)
	{
		return 32;
	}
	else if (_size <= 2024)
	{
		return 64;
	}
	else
	{
		return 128;
	}
}

HashMap::HashMap(unsigned int _size) :
	mMap(new Bucket[_size]),
	mSize(_size),
	mItems(0),
	mNeighborhoodSize(chooseNeighborhoodSize(_size))
{
}

HashMap::~HashMap()
{
	delete[] mMap;
}

bool HashMap::set(const std::string& _key, void* _value)
{
	// Make sure it is not full
	if (mItems == mSize) 
	{
		std::cout << "The map is full" << std::endl;
		return false;
	}

	// Assigns an index based on the hash:
#ifdef DEBUG_HASH
	uint32_t index = fakeHash % mSize;
#else
	uint32_t index = hash(_key) % mSize;
#endif

	//
	// ATTEMPT 1
	// If there is nothing at the bucket for the key value, just insert it there

	if (mMap[index].data == nullptr)
	{
		return insertBucket(_key, _value, index, index);
	}

	// Displays when a collision occurs, interesting information
	std::cout << "Collision with key: " << _key << ", and " << mMap[index].key << std::endl;

	//
	// ATTEMPT 2
	// If there is something there, loop through the "neighborhood" of the bucket 
	
	uint32_t lowerBoundNeighborhood = lowerBound(index);
	uint32_t upperBoundNeighborhood = lowerBoundNeighborhood + mNeighborhoodSize;
	uint32_t searchIndex = lowerBoundNeighborhood;
	for (; searchIndex <= upperBoundNeighborhood; searchIndex++)
	{
		if (mMap[searchIndex].data == nullptr)
		{
			insertBucket(_key, _value, index, searchIndex);
			return true;
		}
	}
	
	//
	// ATTEMPT 3
	// If there is nothing in the neighborhood, use linear proding and search around the neighborhood.
	// A spot is eventually garunteed to be found (map is not empty), then we attempt to move the empty 
	// space into the neighborhood and move it into that empty space.

	// Linearly proding around the neighborhood
	uint32_t emptyBucket;
	uint32_t i = 1;
	while (true)
	{ 
		emptyBucket = i >= lowerBoundNeighborhood ? 0 : lowerBoundNeighborhood - i;
		if (emptyBucket >= 0)
		{
			// empty bucket found
			if (mMap[emptyBucket].data == nullptr)
			{
				break;
			}
		}

		emptyBucket = upperBoundNeighborhood + i;
		if (emptyBucket < mSize)
		{
			// empty bucket found
			if (mMap[emptyBucket].data == nullptr)
			{
				break;
			}
		}

		i++;
	}

	// Attempting to move the empty bucket closer
	uint32_t locCheck = emptyBucket < index ? emptyBucket + 1 : emptyBucket - 1; 

	while (true)
	{
		uint32_t counter = 0;
		// check at least NeighborhoodSize amounts
		for (counter; counter < mNeighborhoodSize; counter++) 
		{
			if (mMap[locCheck].baseBucket == mSize)
			{
				// The previous bucket now points to the next bucket's new location:
				mMap[locCheck + mMap[locCheck].previousOffset].nextOffset += emptyBucket - locCheck;
				mMap[locCheck].previousOffset -= emptyBucket - locCheck;

				// After settings the linked list values, swap:
				mMap[emptyBucket] = mMap[locCheck];
				mMap[locCheck].data = nullptr;
				break;
			}

			// check if the emptyBucket is in the same neighborhood as the locCheck's base, if so, swap.
			if (inNeighborhood(mMap[locCheck].baseBucket, emptyBucket)) 
			{
				mMap[emptyBucket] = mMap[locCheck];
				mMap[locCheck].data = nullptr;
				break;
			}

			if (emptyBucket < index)
			{
				locCheck++;
			}
			else
			{
				locCheck--;
			}
		}

		//
		// ATTEMPT 4
		// No swapping was possible. The last attempt is garunteed to work (or else it is full). Simply add
		// the empty bucket to a linked list starting at the base bucket of "no neighborhood's" buckets.
		// This is a last resort, and has a probability of 1/NeighborhoodSize! occuring.

		if (counter == mNeighborhoodSize) 
		{
			int nextOffset = mMap[index].nextOffset;
			uint32_t currentPosition = index;
			while (mMap[currentPosition].nextOffset != 0)
			{
				currentPosition += mMap[currentPosition].nextOffset;
			}
			mMap[currentPosition].nextOffset = emptyBucket - currentPosition;
			mMap[emptyBucket].previousOffset = -mMap[currentPosition].nextOffset;
			// Base is set to mSize because it does not location-wise belong to a neighborhood (thus, it can be swapped with anything).
			return insertBucket(_key, _value, mSize, emptyBucket, -mMap[currentPosition].nextOffset);
		}
		
		// If an empty bucket was found (Attempt 4 was not needed, yet...) we check if the swap was worth it or not
		// the new empty bucket
		emptyBucket = locCheck;
		// If the empty bucket is in the neighborhood of the base index, we are good.
		if (inNeighborhood(index, emptyBucket))
		{
			return insertBucket(_key, _value, index, emptyBucket);
		}

		// If not, we try again with a new locCheck until Attempt 4 is needed or the empty bucket is within the neighborhood of the base index
		locCheck = emptyBucket < index ? emptyBucket + 1 : emptyBucket - 1;
	}

	return false;
}

uint32_t HashMap::find(const std::string& _key)
{
#ifdef DEBUG_HASH
	uint32_t index = fakeHash % mSize;
#else
	uint32_t index = hash(_key) % mSize;
#endif

	//
	// ATTEMPT 1
	// If all is good, then the original index should hold the value
	if (mMap[index].key == _key)
	{
		return index;
	}

	//
	// ATTEMP 2
	// If not, we search the neighborhood, it is most likely going to be
	// here
	uint32_t searchIndex = lowerBound(index);
	for (int i = 0; i < mNeighborhoodSize + 1; i++)
	{
		if (searchIndex != index) 
		{
			if (mMap[searchIndex].key == _key)
			{
				return searchIndex;
			}
		}
		searchIndex++;
	}

	//
	// ATTEMPT 3
	// If not, then we search the linked list of those who don't belong to a 
	// neighborhood.
	int nextOffset = mMap[index].nextOffset;
	uint32_t currentPosition = index;
	while (mMap[currentPosition].nextOffset != 0)
	{
		if (mMap[currentPosition].key == _key)
		{
			return currentPosition;
		}
		currentPosition += mMap[currentPosition].nextOffset;
	}
	if (mMap[currentPosition].key == _key)
	{
		return currentPosition;
	}

	// If the above attempts don't work, it can't be found, and thus we
	// assign the index and impossible index.
	return mSize;
}