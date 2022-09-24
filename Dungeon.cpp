#include "Camera.hpp"
#include "Decoder.hpp"
#include "Dungeon.hpp"
#include "Logger.hpp"
#include "Window.hpp"

Mesh Dungeon::ShellMesh = Mesh(), Dungeon::WallMesh = Mesh();

//	Finds the largest rectangle in a binary matrix and returns the minimum and maximum expanses. O(sizeX * sizeY) performance.
static size_t FindMaxRectangle(std::vector<std::vector<uint8_t>>& matrix, size_t& indexX, size_t& indexY, size_t& sizeX, size_t& sizeY, const MapTiles tileType)
{
	std::vector<size_t> leftBuffer(matrix.front().size(), 0), rightBuffer(matrix.front().size(), matrix.front().size()), topBuffer(matrix.front().size(), 0), heightBuffer(matrix.front().size(), 0);
	size_t maximalArea = 0, area = 0;

	for (size_t row = 0; row < matrix.size(); ++row)
	{
		size_t curLeft = 0, curRight = matrix.front().size();

		for (size_t column = 0; column < matrix.front().size(); ++column)
		{
			if (matrix[row][column] == tileType)
			{
				heightBuffer[column]++;
				topBuffer[column] = row;
			}

			else
				heightBuffer[column] = 0;
		}

		for (size_t column = 0; column < matrix.front().size(); ++column)
		{
			if (matrix[row][column] == tileType)
				leftBuffer[column] = std::max(leftBuffer[column], curLeft);

			else
			{
				leftBuffer[column] = 0;
				curLeft = column + 1;
			}
		}

		for (int64_t column = static_cast<int64_t>(matrix.front().size()) - 1; column >= 0; --column)
		{
			if (matrix[row][column] == tileType)
				rightBuffer[column] = std::min(rightBuffer[column], curRight);

			else
			{
				rightBuffer[column] = matrix.front().size();
				curRight = column;
			}
		}

		for (size_t column = 0; column < matrix.front().size(); ++column)
		{
			area = (rightBuffer[column] - leftBuffer[column]) * heightBuffer[column];

			if (area > maximalArea)
			{
				maximalArea = area;
				indexX = leftBuffer[column];

				if (topBuffer[column] >= heightBuffer[column])
					indexY = topBuffer[column] - heightBuffer[column] + 1;

				else
					indexY = 0;

				sizeX = rightBuffer[column] - leftBuffer[column];
				sizeY = heightBuffer[column];
			}
		}
	}

	return maximalArea;
}

static void EraseMaxRectangle(std::vector<std::vector<uint8_t>>& cells, const size_t indexX, const size_t indexY, const size_t sizeX, const size_t sizeY, const MapTiles tileType)
{
	for (size_t row = indexY; row < indexY + sizeY; ++row)
		for (size_t column = indexX; column < indexX + sizeX; ++column)
			cells[row][column] = tileType;
}

bool Dungeon::GenerateMesh(const uint16_t sizeX, const uint16_t sizeY, const bool sendToGPU, const bool optimize)
{
	static const float meshScale = 1.33333333f, meshHeight = 1.5f, textureScale = 4.0f;

	std::vector<size_t> pieces;
	std::vector<std::vector<uint8_t>> cells;
	size_t maxArea = 1, indexX = 0, indexY = 0, countX = 0, countY = 0, tileCount = 0;
	float xAlpha = 0.0f, xBeta = 0.0f, zAlpha = 0.0f, zBeta = 0.0f;

	Decoder::Cells.clear();
	Dungeon::ShellMesh.Destroy();
	Dungeon::WallMesh.Destroy();

	if (!Decoder::GenerateMapCode(sizeX, sizeY, &tileCount))
		return Logger::SaveMessage(std::string("Error: GenerateMesh() - 1"), false);

	//Generate a mesh with optimizations that attempts to minimize the number of triangles needed to describe a rectangular dungeon cell grid.
	//This should not generate very many triangle faces. Instead of millions of triangle faces, this may generate tens-to-hundreds of thousands.
	//A one-time wait in this case is a good trade-off for long-term rendering performance and collision-detection performance.
	//Example: ~3.8M triangles generated from an unoptimized 1024x1024 dungeon cell grid vs. ~425k triangles with optimization enabled.
	//TODO: consider batching the grid into four pieces and multi-threading the optimization algorithm (for 512x512 and larger).
	if (optimize)
	{
		cells = Decoder::Cells;
		tileCount = (sizeX + 2) * (sizeY + 2) - tileCount;

		while (tileCount > 0 && maxArea != 0 && tileCount <= (sizeX + 2) * (sizeY + 2))
		{
			maxArea = FindMaxRectangle(cells, indexX, indexY, countX, countY, CLOSED_TILE);
			EraseMaxRectangle(cells, indexX, indexY, countX, countY, OPENED_TILE);
			tileCount -= maxArea;

			pieces.push_back(indexX);
			pieces.push_back(indexY);
			pieces.push_back(countX);
			pieces.push_back(countY);
		}

		xBeta = static_cast<float>(sizeX) * meshScale + meshScale;
		zBeta = static_cast<float>(sizeY) * meshScale + meshScale;

		//Ceiling face of exterior shell of dungeon mesh.
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(xBeta, 0.0f) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(xBeta, zBeta) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(xBeta, zBeta) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zBeta), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, zBeta) / textureScale));

		//Floor face of exterior shell of dungeon mesh.
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(xBeta, zBeta) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zAlpha), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(xBeta, 0.0f) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, zBeta) / textureScale));
		Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(xBeta, zBeta) / textureScale));

		for (size_t index = 0; index < pieces.size(); index += 4)
		{
			xAlpha = static_cast<float>(pieces[index + 1]) * meshScale;
			xBeta = static_cast<float>(pieces[index + 1] + pieces[index + 3]) * meshScale;
			zAlpha = static_cast<float>(pieces[index + 0]) * meshScale;
			zBeta = static_cast<float>(pieces[index + 0] + pieces[index + 2]) * meshScale;

			//Left wall face of exterior shell of dungeon mesh.
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zAlpha), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(zBeta - zAlpha, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(zBeta - zAlpha, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zAlpha), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(zBeta - zAlpha, meshHeight * 2.0f) / textureScale));

			//Front wall face of exterior shell of dungeon mesh.
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(xBeta - xAlpha, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(xBeta - xAlpha, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(xBeta - xAlpha, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zBeta), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, meshHeight * 2.0f) / textureScale));

			//Right wall face of exterior shell of dungeon mesh.
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(zBeta - zAlpha, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zBeta), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(zBeta - zAlpha, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zBeta), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(zBeta - zAlpha, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, meshHeight * 2.0f) / textureScale));

			//Back wall face of exterior shell of dungeon mesh.
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(xBeta - xAlpha, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zAlpha), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(xBeta - xAlpha, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, meshHeight * 2.0f) / textureScale));
			Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(xBeta - xAlpha, meshHeight * 2.0f) / textureScale));
		}
	}

	//Generate a mesh with no optimizations. This can possibly generate millions of triangle faces depending on the size of the dungeon cell grid.
	else
	{
		for (size_t row = 1; row < Decoder::Cells.size() - 1; ++row)
		{
			xAlpha = static_cast<float>(row - 1) * meshScale;
			xBeta = xAlpha + meshScale;

			for (size_t column = 1; column < Decoder::Cells.front().size() - 1; ++column)
			{
				if (Decoder::Cells[row][column] == OPENED_TILE)
				{
					zAlpha = static_cast<float>(column - 1) * meshScale;
					zBeta = zAlpha + meshScale;

					//Ceiling face of exterior shell of dungeon mesh.
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zBeta), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));

					//Floor face of exterior shell of dungeon mesh.
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zAlpha), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
					Dungeon::ShellMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)));

					if (Decoder::Cells[row - 1][column] == CLOSED_TILE)
					{
						//Left wall face of exterior shell of dungeon mesh.
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta - meshScale, -meshHeight, zAlpha), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta - meshScale, meshHeight, zBeta), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta - meshScale, -meshHeight, zBeta), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta - meshScale, -meshHeight, zAlpha), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta - meshScale, meshHeight, zAlpha), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta - meshScale, meshHeight, zBeta), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
					}

					if (Decoder::Cells[row][column - 1] == CLOSED_TILE)
					{
						//Front wall face of exterior shell of dungeon mesh.
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta - meshScale), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zBeta - meshScale), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta - meshScale), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zBeta - meshScale), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zBeta - meshScale), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zBeta - meshScale), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)));
					}

					if (Decoder::Cells[row + 1][column] == CLOSED_TILE)
					{
						//Right wall face of exterior shell of dungeon mesh.
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha + meshScale, -meshHeight, zAlpha), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha + meshScale, -meshHeight, zBeta), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha + meshScale, meshHeight, zBeta), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha + meshScale, -meshHeight, zAlpha), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha + meshScale, meshHeight, zBeta), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha + meshScale, meshHeight, zAlpha), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)));
					}

					if (Decoder::Cells[row][column + 1] == CLOSED_TILE)
					{
						//Back wall face of exterior shell of dungeon mesh.
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha + meshScale), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha + meshScale), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, -meshHeight, zAlpha + meshScale), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -meshHeight, zAlpha + meshScale), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xAlpha, meshHeight, zAlpha + meshScale), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)));
						Dungeon::WallMesh.AddVertex(Vertex(glm::vec3(xBeta, meshHeight, zAlpha + meshScale), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)));
					}
				}
			}
		}
	}

	if (sendToGPU && (!Dungeon::ShellMesh.Create() || !Dungeon::WallMesh.Create()))
	{
		Dungeon::ShellMesh.Destroy();
		Dungeon::WallMesh.Destroy();

		return Logger::SaveMessage(std::string("Error: GenerateMesh() - 2"), false);
	}

	return true;
}

bool Dungeon::RenderShellMesh(const Shader& shader)
{
	if (!Dungeon::ShellMesh.Render(shader))
		return Logger::SaveMessage(std::string("Error: RenderShellMesh() - 1"), false);

	return true;
}

bool Dungeon::RenderWallMesh(const Shader& shader)
{
	if (!Dungeon::WallMesh.Render(shader))
		return Logger::SaveMessage(std::string("Error: RenderWallMesh() - 1"), false);

	return true;
}

bool Dungeon::LockCamera()
{
	//TODO: Make this synchronized with other declarations.
	static const float meshScale = 1.33333333f;
	float minX = 0.0f, minY = 0.0f, clampX = 0.0f, clampY = 0.0f, distance = 0.0f;

	if (Decoder::Cells.empty())
		return Logger::SaveMessage(std::string("Error: LockCamera() - 1"), false);

	for (size_t indexX = 0; indexX < Decoder::Cells.size(); ++indexX)
		for (size_t indexY = 0; indexY < Decoder::Cells.front().size(); ++indexY)
			if (Decoder::Cells[indexX][indexY] == CLOSED_TILE)
			{
				minX = static_cast<float>(indexX) * meshScale;
				minY = static_cast<float>(indexY) * meshScale;
				clampX = glm::clamp(Camera::Position.x, minX, minX + meshScale);
				clampY = glm::clamp(Camera::Position.z, minY, minY + meshScale);
				distance = glm::sqrt((clampX - Camera::Position.x) * (clampX - Camera::Position.x) + (clampY - Camera::Position.z) * (clampY - Camera::Position.z));

				if (distance < 0.25f * meshScale)
					Camera::Position -= glm::vec3(clampX - Camera::Position.x, 0.0f, clampY - Camera::Position.z) * Window::DeltaTime(32.0f) * meshScale;
			}

	return true;
}

bool Dungeon::DestroyMesh()
{
	if (!Dungeon::ShellMesh.Destroy())
	{
		Dungeon::WallMesh.Destroy();
		return Logger::SaveMessage(std::string("Error: DestroyMesh() - 1"), false);
	}

	else if (!Dungeon::WallMesh.Destroy())
		return Logger::SaveMessage(std::string("Error: DestroyMesh() - 2"), false);

	return true;
}