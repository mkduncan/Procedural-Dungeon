#include "Decoder.hpp"
#include "Dungeon.hpp"
#include "Logger.hpp"
#include "StaticMap.hpp"

#include <chrono>
#include <fstream>
#include <limits>
#include <random>
#include <sstream>

#include "lzma/Alloc.h"
#include "lzma/LzmaDec.h"
#include "lzma/LzmaEnc.h"

//	Generates a high quality pseudo-random number.
static size_t NextRandomIndex();
static int32_t NextRandom();

#define STB_HERRINGBONE_WANG_TILE_IMPLEMENTATION
#define STB_HBWANG_ASSERT
#define STB_HBWANG_MAX_X 1024
#define STB_HBWANG_MAX_Y 1024
#define STB_HBWANG_RAND() NextRandom()

#include "Libraries/stb/stb_herringbone_wang_tile.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_HDR

#include "Libraries/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_ASSERT

#include "Libraries/stb/stb_image_write.h"

std::vector<std::vector<uint8_t>> Decoder::Cells = std::vector<std::vector<uint8_t>>();
std::vector<uint8_t> Decoder::Buffer = std::vector<uint8_t>();
size_t Decoder::Width = 0, Decoder::Height = 0, Decoder::Depth = 0;

//	Reads an image file with a special format used for generating a dungeon map.
bool Decoder::ReadSourceImage(const std::string& imagePath)
{
	int32_t sizeX = 0, sizeY = 0, sizeZ = 0;
	uint8_t* buffer = stbi_load(imagePath.c_str(), &sizeX, &sizeY, &sizeZ, 3);

	if (buffer == nullptr)
	{
		Decoder::ClearBuffers();
		return Logger::SaveMessage("Error: Decoder::ReadSourceImage() - 1");
	}

	Decoder::Width = static_cast<size_t>(sizeX);
	Decoder::Height = static_cast<size_t>(sizeY);
	Decoder::Depth = static_cast<size_t>(sizeZ);
	Decoder::Buffer.resize(Decoder::Width * Decoder::Height * Decoder::Depth);
	memcpy(&Decoder::Buffer[0], &buffer[0], Buffer.size());
	stbi_image_free(buffer);

	return true;
}

//	Generates a dungeon map using the "Herringbone-Wang" algorithm as described by "stb-nothings".
bool Decoder::GenerateMapImage(const uint16_t sizeX, const uint16_t sizeY)
{
	stbhw_tileset tiles;

	if (Decoder::Buffer.empty())
		return Logger::SaveMessage("Error: Decoder::GenerateMapImage() - 1");

	else if (sizeX < 32 || sizeX > STB_HBWANG_MAX_X || sizeY < 32 || sizeY > STB_HBWANG_MAX_Y)
		return Logger::SaveMessage("Error: Decoder::GenerateMapImage() - 2");

	else if (stbhw_build_tileset_from_image(&tiles, &Decoder::Buffer[0], static_cast<int32_t>(Decoder::Width * Decoder::Depth), static_cast<int32_t>(Decoder::Width), static_cast<int32_t>(Decoder::Height)) == 0)
		return Logger::SaveMessage("Error: Decoder::GenerateMapImage() - 3");

	Decoder::Width = static_cast<size_t>(sizeX);
	Decoder::Height = static_cast<size_t>(sizeY);
	Decoder::Buffer.resize(Decoder::Width * Decoder::Height * Decoder::Depth);

	if (stbhw_generate_image(&tiles, nullptr, &Decoder::Buffer[0], static_cast<int32_t>(Decoder::Width * Decoder::Depth), static_cast<int32_t>(Decoder::Width), static_cast<int32_t>(Decoder::Height)) == 0)
	{
		stbhw_free_tileset(&tiles);
		Decoder::ClearBuffers();

		return Logger::SaveMessage("Error: Decoder::GenerateMapImage() - 4");
	}

	stbhw_free_tileset(&tiles);
	return true;
}

//	Writes a generated dungeon map to an image file.
bool Decoder::WriteMapImage(const std::string& imagePath)
{
	int32_t result = 0;

	if (Decoder::Buffer.empty())
		return Logger::SaveMessage("Error: Decoder::WriteMapImage() - 1");

	result = stbi_write_png(imagePath.c_str(), static_cast<int32_t>(Decoder::Width), static_cast<int32_t>(Decoder::Height), static_cast<int32_t>(Decoder::Depth), &Decoder::Buffer[0], static_cast<int32_t>(Decoder::Width * Decoder::Depth));
	Decoder::ClearBuffers();

	if (result == 0)
		return Logger::SaveMessage("Error: Decoder::WriteMapImage() - 2");

	return true;
}

//	Generates a source code file that contains the compressed raw image pixel data of an image with a special format used for generating dungeon maps.
bool Decoder::GenerateStaticCode(const std::string& codePath)
{
	std::ofstream output;
	std::vector<uint8_t> compressed;
	std::string headerFile = codePath + ".hpp", sourceFile = codePath + ".cpp", outputBuffer;
	CLzmaEncProps props;
	size_t propsSize, compressedSize = ((Decoder::Buffer.size() * 5) >> 2) + 256;

	if (Decoder::Buffer.empty())
		return Logger::SaveMessage("Error: Decoder::GenerateStaticCode() - 1");

	compressed.resize(compressedSize);
	LzmaEncProps_Init(&props);

	props.level = 9;
	props.fb = 273;
	props.numThreads = 1;

	if (LzmaEncode(&compressed[LZMA_PROPS_SIZE], &compressedSize, &Decoder::Buffer[0], Decoder::Buffer.size(), &props, &compressed[0], &propsSize, props.writeEndMark, nullptr, &g_Alloc, &g_MidAlloc) != SZ_OK)
		return Logger::SaveMessage("Error: Decoder::GenerateStaticCode() - 2");

	compressed.resize(propsSize + compressedSize);
	output.open(headerFile.c_str());

	if (!output)
		return Logger::SaveMessage("Error: Decoder::GenerateStaticCode() - 3");

	outputBuffer += "//This source file was generated automatically by the \"Dungeon\" project's \"Decoder\" class using the \"GenerateStaticCode\" method.\n";
	outputBuffer += "//The purpose of this source file is to store compressed data used for algorithmically generating dungeon maps.\n\n";
	outputBuffer += "#ifndef StaticMap_H\n";
	outputBuffer += "#define StaticMap_H\n\n";
	outputBuffer += "#include <cstdint>\n";
	outputBuffer += "#include <vector>\n\n";
	outputBuffer += "class StaticMap\n";
	outputBuffer += "{\n";
	outputBuffer += "private:\n";
	outputBuffer += "\tStaticMap() = delete;\n";
	outputBuffer += "\t~StaticMap() = delete;\n";
	outputBuffer += "\tStaticMap(const StaticMap&) = delete;\n";
	outputBuffer += "\tStaticMap(StaticMap&&) = delete;\n";
	outputBuffer += "\tStaticMap& operator=(const StaticMap&) = delete;\n";
	outputBuffer += "\tStaticMap& operator=(StaticMap&&) = delete;\n\n";
	outputBuffer += "public:\n";
	outputBuffer += "\tstatic std::vector<uint8_t> Buffer;\n";
	outputBuffer += "\tstatic size_t Width, Height, Depth;\n";
	outputBuffer += "};\n\n";
	outputBuffer += "#endif";

	output << outputBuffer << std::endl;
	outputBuffer.clear();
	output.close();
	output.open(sourceFile.c_str());

	if (!output)
		return Logger::SaveMessage("Error: Decoder::GenerateStaticCode() - 4");

	outputBuffer += "//This source file was generated automatically by the \"Dungeon\" project's \"Decoder\" class using the \"GenerateStaticCode\" method.\n";
	outputBuffer += "//The purpose of this source file is to store compressed data used for algorithmically generating dungeon maps.\n\n";
	outputBuffer += "#include \"StaticMap.hpp\"\n\n";
	outputBuffer += "std::vector<uint8_t> StaticMap::Buffer =\n";
	outputBuffer += "{";

	for (size_t index = 0; index < compressed.size(); ++index)
	{
		if ((index & 63) == 0)
			outputBuffer += "\n\t";

		outputBuffer += std::to_string(compressed[index]);

		if (index + 1 < compressed.size())
			outputBuffer += ", ";
	}

	compressed.clear();
	outputBuffer += "\n};\n\n";
	outputBuffer += "size_t StaticMap::Width = " + std::to_string(Decoder::Width) + ";\n";
	outputBuffer += "size_t StaticMap::Height = " + std::to_string(Decoder::Height) + ";\n";
	outputBuffer += "size_t StaticMap::Depth = " + std::to_string(Decoder::Depth) + ";";
	output << outputBuffer << std::endl;
	output.close();

	return true;
}

//	Generates a dungeon map using the compressed raw image pixel data that was written to a source code file using the "GenerateStaticCode" method above.
bool Decoder::GenerateMapCode(const uint16_t sizeX, const uint16_t sizeY, size_t* tileCount)
{
	std::vector<uint8_t> decompressed;
	ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
	SizeT compressedSize = StaticMap::Buffer.size() - LZMA_PROPS_SIZE, decompressedSize = 0, index = 0;

	if (StaticMap::Buffer.empty())
		return Logger::SaveMessage("Error: Decoder::GenerateMapCode() - 1");

	else if (sizeX < 32 || sizeX > STB_HBWANG_MAX_X || sizeY < 32 || sizeY > STB_HBWANG_MAX_Y)
		return Logger::SaveMessage("Error: Decoder::GenerateMapCode() - 2");

	decompressed.resize(StaticMap::Width * StaticMap::Height * StaticMap::Depth);
	decompressedSize = decompressed.size();

	if (LzmaDecode(&decompressed[0], &decompressedSize, &StaticMap::Buffer[LZMA_PROPS_SIZE], &compressedSize, &StaticMap::Buffer[0], LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &g_Alloc) != SZ_OK)
		return Logger::SaveMessage("Error: Decoder::GenerateMapCode() - 3");

	Decoder::Buffer = decompressed;
	decompressed.clear();
	Decoder::Width = StaticMap::Width;
	Decoder::Height = StaticMap::Height;
	Decoder::Depth = StaticMap::Depth;

	if (!Decoder::GenerateMapImage(sizeX, sizeY))
	{
		Decoder::ClearBuffers();
		return Logger::SaveMessage("Error: Decoder::GenerateMapCode() - 4");
	}

	Decoder::Cells = std::vector<std::vector<uint8_t>>(sizeX + 2, std::vector<uint8_t>(sizeY + 2, CLOSED_TILE));

	for (size_t row = 1; row < sizeX + 1; ++row)
		for (size_t column = 1; column < sizeY + 1; ++column)
		{
			if (Decoder::Buffer[index] != 255 || Decoder::Buffer[index + 1] != 255 || Decoder::Buffer[index + 2] != 255)
			{
				Decoder::Cells[row][column] = OPENED_TILE;

				if (tileCount != nullptr)
					++(*tileCount);
			}

			index += 3;
		}

	Decoder::ClearBuffers();
	return true;
}

//	Writes an image file from the generated dungeon map using the "GenerateMapCode" method above.
bool Decoder::WriteMapCode(const std::string& imagePath)
{
	size_t index = 0;
	int32_t result = 0;

	if (Decoder::Cells.empty())
		return Logger::SaveMessage("Error: Decoder::WriteMapCode() - 1");

	Decoder::Width = Decoder::Cells.size() - 2;
	Decoder::Height = Decoder::Cells.front().size() - 2;
	Decoder::Depth = 3;
	Decoder::Buffer.resize(Decoder::Width * Decoder::Height * Decoder::Depth);

	for (size_t row = 1; row < Decoder::Cells.size() - 1; ++row)
	{
		for (size_t column = 1; column < Decoder::Cells.front().size() - 1; ++column)
		{
			if (Decoder::Cells[row][column] == CLOSED_TILE)
				memset(reinterpret_cast<void*>(&Decoder::Buffer[index]), 0, Decoder::Depth);

			else
				memset(reinterpret_cast<void*>(&Decoder::Buffer[index]), 255, Decoder::Depth);

			index += 3;
		}
	}

	result = stbi_write_png(imagePath.c_str(), static_cast<int32_t>(Decoder::Width), static_cast<int32_t>(Decoder::Height), static_cast<int32_t>(Decoder::Depth), &Decoder::Buffer[0], static_cast<int32_t>(Decoder::Width * Decoder::Depth));
	Decoder::ClearBuffers();

	if (result == 0)
		return Logger::SaveMessage("Error: Decoder::WriteMapCode() - 2");

	return true;
}

//	Generates a model file to be used by external applications for viewing the 2D dungeon map projected into 3D space.
bool Decoder::WriteModelCode(const std::string& modelFile)
{
	std::stringstream outputBuffer, vertexBuffer, normalBuffer, bitmapBuffer, faceBuffer;
	std::ofstream output;
	Mesh mesh;
	std::string materialPath = modelFile + ".mtl", modelPath = modelFile + ".obj";
	std::streamsize digits = 4;
	size_t faceIndex = 1;

	if (Decoder::Cells.empty())
		return Logger::SaveMessage("Error: Decoder::WriteModelCode() - 1");

	output.open(materialPath.c_str());

	if (!output)
		return Logger::SaveMessage("Error: Decoder::WriteModelCode() - 2");

	outputBuffer.precision(digits);
	outputBuffer << std::fixed;
	outputBuffer << "#\tThis material file was automatically generated by computer software.\n";
	outputBuffer << "#\tThis material is only intended to be used for testing and debugging purposes.\n\n";
	outputBuffer << "newmtl Material\n";
	outputBuffer << "Ns " << 360.0f << "\n";
	outputBuffer << "Ka " << 1.0f << ' ' << 1.0f << ' ' << 1.0f << "\n";
	outputBuffer << "Kd " << 0.8f << ' ' << 0.8f << ' ' << 0.8f << "\n";
	outputBuffer << "Ks " << 0.5f << ' ' << 0.5f << ' ' << 0.5f << "\n";
	outputBuffer << "Ke " << 0.0f << ' ' << 0.0f << ' ' << 0.0f << "\n";
	outputBuffer << "Ni " << 1.45f << "\n";
	outputBuffer << "d " << 1.0f << "\n";
	outputBuffer << "illum 2\n";
	outputBuffer << "map_Kd Assets/rock_texture.png";

	output << outputBuffer.str() << std::endl;
	outputBuffer = std::stringstream();
	output.close();
	output.open(modelPath.c_str());

	if (!output)
		return Logger::SaveMessage("Error: Decoder::WriteModelCode() - 3");

	vertexBuffer.precision(digits);
	normalBuffer.precision(digits);
	bitmapBuffer.precision(digits);
	vertexBuffer << std::fixed;
	normalBuffer << std::fixed;
	bitmapBuffer << std::fixed;

	Decoder::Width = Decoder::Cells.size() - 2;
	Decoder::Height = Decoder::Cells.front().size() - 2;

	if (!Dungeon::GenerateMesh(static_cast<uint16_t>(Decoder::Width), static_cast<uint16_t>(Decoder::Height), mesh))
	{
		output.close();
		return Logger::SaveMessage("Error: Decoder::WriteModelCode() - 4");
	}

	outputBuffer << "#\tThis object file was automatically generated by computer software.\n";
	outputBuffer << "#\tThis 3D model is only intended to be used for testing and debugging purposes.\n\n";
	outputBuffer << "mtllib " << materialPath << "\n";
	outputBuffer << "o Dungeon\n";

	for (size_t index = 0; index < mesh.Vertices.size(); ++index)
	{
		vertexBuffer << "v " << mesh.Vertices[index].Position.x << " " << mesh.Vertices[index].Position.y << " " << mesh.Vertices[index].Position.z << "\n";
		normalBuffer << "vn " << mesh.Vertices[index].Normal.x << " " << mesh.Vertices[index].Normal.y << " " << mesh.Vertices[index].Normal.z << "\n";
		bitmapBuffer << "vt " << mesh.Vertices[index].Texture.x << " " << mesh.Vertices[index].Texture.y << "\n";

		if (index % 3 == 0)
		{
			faceBuffer << "f " << faceIndex << "/" << faceIndex << "/" << faceIndex << " ";
			++faceIndex;
			faceBuffer << faceIndex << "/" << faceIndex << "/" << faceIndex << " ";
			++faceIndex;
			faceBuffer << faceIndex << "/" << faceIndex << "/" << faceIndex << "\n";
			++faceIndex;
		}
	}

	outputBuffer << vertexBuffer.str();
	vertexBuffer = std::stringstream();
	outputBuffer << bitmapBuffer.str();
	bitmapBuffer = std::stringstream();
	outputBuffer << normalBuffer.str();
	normalBuffer = std::stringstream();
	outputBuffer << "g Dungeon_Dungeon_Material\n";
	outputBuffer << "usemtl Material\n";
	outputBuffer << "s off\n";
	outputBuffer << faceBuffer.str();
	faceBuffer = std::stringstream();
	output << outputBuffer.str() << std::endl;
	outputBuffer = std::stringstream();
	output.close();
	Decoder::ClearBuffers();

	return true;
}

void Decoder::ClearBuffers()
{
	Decoder::Buffer.clear();
	Decoder::Width = 0;
	Decoder::Height = 0;
	Decoder::Depth = 0;
}

//	Generates a high quality pseudo-random number.
static size_t NextRandomIndex()
{
	static std::mt19937_64 generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	return generator();
}

//	Generates a high quality pseudo-random number with a width of four bytes.
static int32_t NextRandom()
{
	return NextRandomIndex() % std::numeric_limits<int32_t>::max();
}