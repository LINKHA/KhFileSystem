#include "StringHash.h"

#include <cstdio>

namespace KhFileSystem
{

const FStringHash FStringHash::ZERO;

/// Update a hash with the given 8-bit value using the SDBM algorithm.
inline unsigned FStringHash_SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6u) + (hash << 16u) - hash; }

FString FStringHash::ToString() const
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%08X", _value);
    return FString(tempBuffer);
}

unsigned FStringHash::Calculate(const char* str, unsigned hash)
{
	if (!str)
		return hash;

	while (*str)
	{
		hash = FStringHash_SDBMHash(hash, (unsigned char)*str++);
	}

	return hash;
}

}