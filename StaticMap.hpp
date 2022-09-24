//This source file was generated automatically by the "Dungeon" project's "Decoder" class using the "GenerateStaticCode" method.
//The purpose of this source file is to store compressed data used for algorithmically generating dungeon maps.

#ifndef StaticMap_H
#define StaticMap_H

#include <cstdint>
#include <vector>

class StaticMap
{
private:
	StaticMap() = delete;
	~StaticMap() = delete;
	StaticMap(const StaticMap&) = delete;
	StaticMap(StaticMap&&) = delete;
	StaticMap& operator=(const StaticMap&) = delete;
	StaticMap& operator=(StaticMap&&) = delete;

public:
	static std::vector<uint8_t> Buffer;
	static size_t Width, Height, Depth;
};

#endif
