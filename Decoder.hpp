#ifndef Decoder_H
#define Decoder_H

#include <cstdint>
#include <string>
#include <vector>

enum MapTiles
{
	OPENED_TILE = ' ',
	CLOSED_TILE = 'O'
};

//	This class is mostly used for testing and debugging purposes. This class encapsulates all of the methods involved in generating a dungeon
//	map using the "Herringbone-Wang" algorithm as described and written by "stb-nothings". The only method in this class that is used for
//	more than testing and debugging purposes is the "GenerateMapCode" method, which generates a numeric array of values that indicates where
//	things are located in the generated dungeon map.
class Decoder
{
private:
	Decoder() = delete;
	~Decoder() = delete;
	Decoder(const Decoder&) = delete;
	Decoder(Decoder&&) noexcept = delete;
	Decoder& operator=(const Decoder&) = delete;
	Decoder& operator=(Decoder&&) noexcept = delete;

public:
	static std::vector<std::vector<uint8_t>> Cells;
	static std::vector<uint8_t> Buffer;
	static size_t Width, Height, Depth;

	static bool ReadSourceImage(const std::string&);
	static bool GenerateMapImage(const uint16_t, const uint16_t);
	static bool WriteMapImage(const std::string&);
	static bool GenerateStaticCode(const std::string&);
	static bool GenerateMapCode(const uint16_t, const uint16_t, size_t* = nullptr);
	static bool WriteMapCode(const std::string&);
	static bool WriteModelCode(const std::string&);
	static void ClearBuffers();
};

//	Example code to read from a source image file with a special tile mapping format, generate source code that declares a static array
//	containing the compressed raw image pixel data, generate an image file of a randomly tiled dungeon map based from the source image
//	file, output the dungeon map to an image file, generate a randomly tiled dungeon map as a numeric array of tiled values based on the
//	compressed raw image pixel data in the generated source code file, output an image based on the numeric array of tiled values, and
//	output a 3D model file of the generated dungeon map.
//
//	Decoder::ReadSourceImage("chunks.png");
//	Decoder::GenerateStaticCode("StaticMap");
//	Decoder::GenerateMapImage(128, 128);
//	Decoder::WriteMapImage("output.png");
//	Decoder::GenerateMapCode(128, 128);
//	Decoder::WriteMapCode("output_map.png");
//	Decoder::WriteModelCode("output");
//	Logger::PrintMessages("output.txt");

#endif