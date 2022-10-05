#include "Camera.hpp"
#include "Decoder.hpp"
#include "Dungeon.hpp"
#include "Logger.hpp"
#include "StaticShader.hpp"
#include "Window.hpp"

//	Finds the largest rectangle in a binary matrix and returns the minimum and maximum expanses. O(sizeX * sizeY) performance.
static size_t FindMaxRectangle(std::vector<std::vector<uint8_t>>&, size_t&, size_t&, size_t&, size_t&, const MapTiles);
static void EraseMaxRectangle(std::vector<std::vector<uint8_t>>&, const size_t, const size_t, const size_t, const size_t, const MapTiles);

float Dungeon::MeshScale = 1.33333333f;
float Dungeon::MeshHeight = 1.5f;
float Dungeon::TextureScale = 0.25f;

//  Generate a mesh with optimizations that attempts to minimize the number of triangles needed to describe a rectangular dungeon cell grid.
//  This should not generate as many triangle faces. Instead of millions of triangle faces, this may generate hundreds of thousands.
//  A one-time wait in this case is a good trade-off for long-term rendering performance and collision-detection performance.
//  Example: ~3.8M triangles generated from an unoptimized 1024x1024 dungeon cell grid vs. ~425k triangles with optimization enabled.
//  TODO: consider batching the grid into four pieces and multi-threading the optimization algorithm (384x384 and larger - noticeably slow).
bool Dungeon::GenerateModel(const uint16_t sizeX, const uint16_t sizeY, Model& model, std::vector<AlignedBox>& boxes)
{
	Mesh floorMesh, wallMesh;
	std::vector<std::vector<uint8_t>> cells;
	size_t maxArea = 1, indexX = 0, indexY = 0, countX = 0, countY = 0, tileCount = 0;
	float xAlpha = 0.0f, xBeta = 0.0f, zAlpha = 0.0f, zBeta = 0.0f;

	if (!model.Destroy())
		return Logger::SaveMessage(std::string("Error: Dungeon::GenerateModel() - 1."));

	Decoder::Cells.clear();
	boxes.clear();

	if (!Decoder::GenerateMapCode(sizeX, sizeY, &tileCount))
		return Logger::SaveMessage(std::string("Error: Dungeon::GenerateModel() - 2."));

	cells = Decoder::Cells;
	tileCount = (sizeX + 2) * (sizeY + 2) - tileCount;

	//Decomposes all non-occupied cells (areas the camera cannot access due to walls) into a greedy maximally-sized set of rectangles.
	//These rectangles are used to generate wall boundaries (and also used to minimize the number of wall-triangles rendered).
	//Due to the rectangular nature of the generated dungeon, these rectangles can be used for simple collision detection by using
	//numerous sphere-box intersection tests. These can also be used for AI path-finding and occlusion in rendering (TODO).
	while (tileCount > 0 && maxArea != 0 && tileCount <= (sizeX + 2) * (sizeY + 2))
	{
		maxArea = FindMaxRectangle(cells, indexX, indexY, countX, countY, CLOSED_TILE);
		EraseMaxRectangle(cells, indexX, indexY, countX, countY, OPENED_TILE);
		tileCount -= maxArea;
		boxes.emplace_back();
		boxes.back().Envelope(glm::vec3(static_cast<float>(indexX), -Dungeon::MeshHeight, static_cast<float>(indexY)));
		boxes.back().Envelope(glm::vec3(static_cast<float>(indexX + countX), Dungeon::MeshHeight, static_cast<float>(indexY + countY)));
	}

	if (tileCount != 0)
	{
		boxes.clear();
		return Logger::SaveMessage(std::string("Error: Dungeon::GenerateModel() - 3."));
	}

	xBeta = static_cast<float>(sizeX + 1) * Dungeon::MeshScale;
	zBeta = static_cast<float>(sizeY + 1) * Dungeon::MeshScale;

	//Ceiling face of exterior shell of dungeon mesh.
	floorMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha), Vertex::Down, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha), Vertex::Down, glm::vec2(xBeta, 0.0f) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Down, glm::vec2(xBeta, zBeta) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha), Vertex::Down, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Down, glm::vec2(xBeta, zBeta) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zBeta), Vertex::Down, glm::vec2(0.0f, zBeta) * Dungeon::TextureScale));

	//Floor face of exterior shell of dungeon mesh.
	floorMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::Up, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta), Vertex::Up, glm::vec2(xBeta, zBeta) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zAlpha), Vertex::Up, glm::vec2(xBeta, 0.0f) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::Up, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta), Vertex::Up, glm::vec2(0.0f, zBeta) * Dungeon::TextureScale));
	floorMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta), Vertex::Up, glm::vec2(xBeta, zBeta) * Dungeon::TextureScale));

	for (size_t index = 0; index < boxes.size(); ++index)
	{
		xAlpha = boxes[index].Minimum.z * Dungeon::MeshScale;
		xBeta = boxes[index].Maximum.z * Dungeon::MeshScale;
		zAlpha = boxes[index].Minimum.x * Dungeon::MeshScale;
		zBeta = boxes[index].Maximum.x * Dungeon::MeshScale;

		//Left wall face of exterior shell of dungeon mesh.
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zAlpha), Vertex::Right, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Right, glm::vec2(zBeta - zAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta), Vertex::Right, glm::vec2(zBeta - zAlpha, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zAlpha), Vertex::Right, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha), Vertex::Right, glm::vec2(0.0f, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Right, glm::vec2(zBeta - zAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));

		//Front wall face of exterior shell of dungeon mesh.
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta), Vertex::Out, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta), Vertex::Out, glm::vec2(xBeta - xAlpha, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Out, glm::vec2(xBeta - xAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta), Vertex::Out, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Out, glm::vec2(xBeta - xAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zBeta), Vertex::Out, glm::vec2(0.0f, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));

		//Right wall face of exterior shell of dungeon mesh.
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::Left, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta), Vertex::Left, glm::vec2(zBeta - zAlpha, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zBeta), Vertex::Left, glm::vec2(zBeta - zAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::Left, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zBeta), Vertex::Left, glm::vec2(zBeta - zAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha), Vertex::Left, glm::vec2(0.0f, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));

		//Back wall face of exterior shell of dungeon mesh.
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::In, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha), Vertex::In, glm::vec2(xBeta - xAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zAlpha), Vertex::In, glm::vec2(xBeta - xAlpha, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::In, glm::vec2(0.0f, 0.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha), Vertex::In, glm::vec2(0.0f, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));
		wallMesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha), Vertex::In, glm::vec2(xBeta - xAlpha, Dungeon::MeshHeight * 2.0f) * Dungeon::TextureScale));

		boxes[index].Minimum.x = zAlpha;
		boxes[index].Minimum.z = xAlpha;
		boxes[index].Maximum.x = zBeta;
		boxes[index].Maximum.z = xBeta;
		boxes[index].Id = index * 24;
	}

	model.Meshes.resize(2);
	model.Meshes.front().LoadMesh(floorMesh);
	model.Meshes.back().LoadMesh(wallMesh);

	return true;
}

//  Generate a mesh with no optimizations. This can possibly generate millions of triangle faces depending on
//  the size of the dungeon cell grid. This is used in 3D model file generation for testing purposes.
bool Dungeon::GenerateMesh(const uint16_t sizeX, const uint16_t sizeY, Mesh &mesh)
{
	std::vector<size_t> pieces;
	std::vector<std::vector<uint8_t>> cells;
	size_t maxArea = 1, indexX = 0, indexY = 0, countX = 0, countY = 0, tileCount = 0;
	float xAlpha = 0.0f, xBeta = 0.0f, zAlpha = 0.0f, zBeta = 0.0f;

	Decoder::Cells.clear();

	if (!mesh.Destroy())
		return Logger::SaveMessage(std::string("Error: GenerateMesh() - 1"));

	if (!Decoder::GenerateMapCode(sizeX, sizeY, &tileCount))
		return Logger::SaveMessage(std::string("Error: GenerateMesh() - 2"));

	for (size_t row = 1; row < Decoder::Cells.size() - 1; ++row)
	{
		xAlpha = static_cast<float>(row) * Dungeon::MeshScale;
		xBeta = xAlpha + Dungeon::MeshScale;

		for (size_t column = 1; column < Decoder::Cells.front().size() - 1; ++column)
		{
			if (Decoder::Cells[row][column] == OPENED_TILE)
			{
				zAlpha = static_cast<float>(column) * Dungeon::MeshScale;
				zBeta = zAlpha + Dungeon::MeshScale;

				//Ceiling face of exterior shell of dungeon mesh.
				mesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha), Vertex::Down, glm::vec2(0.0f, 0.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha), Vertex::Down, glm::vec2(1.0f, 0.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Down, glm::vec2(1.0f, 1.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha), Vertex::Down, glm::vec2(0.0f, 0.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta), Vertex::Down, glm::vec2(1.0f, 1.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zBeta), Vertex::Down, glm::vec2(0.0f, 1.0f)));

				//Floor face of exterior shell of dungeon mesh.
				mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::Up, glm::vec2(0.0f, 0.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta), Vertex::Up, glm::vec2(1.0f, 1.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zAlpha), Vertex::Up, glm::vec2(1.0f, 0.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha), Vertex::Up, glm::vec2(0.0f, 0.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta), Vertex::Up, glm::vec2(0.0f, 1.0f)));
				mesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta), Vertex::Up, glm::vec2(1.0f, 1.0f)));

				if (Decoder::Cells[row - 1][column] == CLOSED_TILE)
				{
					//Left wall face of exterior shell of dungeon mesh.
					mesh.AddVertex(Vertex(glm::vec3(xBeta - Dungeon::MeshScale, -Dungeon::MeshHeight, zAlpha), Vertex::Right, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta - Dungeon::MeshScale, Dungeon::MeshHeight, zBeta), Vertex::Right, glm::vec2(1.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta - Dungeon::MeshScale, -Dungeon::MeshHeight, zBeta), Vertex::Right, glm::vec2(1.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta - Dungeon::MeshScale, -Dungeon::MeshHeight, zAlpha), Vertex::Right, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta - Dungeon::MeshScale, Dungeon::MeshHeight, zAlpha), Vertex::Right, glm::vec2(0.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta - Dungeon::MeshScale, Dungeon::MeshHeight, zBeta), Vertex::Right, glm::vec2(1.0f, 1.0f)));
				}

				if (Decoder::Cells[row][column - 1] == CLOSED_TILE)
				{
					//Front wall face of exterior shell of dungeon mesh.
					mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta - Dungeon::MeshScale), Vertex::Out, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zBeta - Dungeon::MeshScale), Vertex::Out, glm::vec2(1.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta - Dungeon::MeshScale), Vertex::Out, glm::vec2(1.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zBeta - Dungeon::MeshScale), Vertex::Out, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zBeta - Dungeon::MeshScale), Vertex::Out, glm::vec2(1.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zBeta - Dungeon::MeshScale), Vertex::Out, glm::vec2(0.0f, 1.0f)));
				}

				if (Decoder::Cells[row + 1][column] == CLOSED_TILE)
				{
					//Right wall face of exterior shell of dungeon mesh.
					mesh.AddVertex(Vertex(glm::vec3(xAlpha + Dungeon::MeshScale, -Dungeon::MeshHeight, zAlpha), Vertex::Left, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha + Dungeon::MeshScale, -Dungeon::MeshHeight, zBeta), Vertex::Left, glm::vec2(1.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha + Dungeon::MeshScale, Dungeon::MeshHeight, zBeta), Vertex::Left, glm::vec2(1.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha + Dungeon::MeshScale, -Dungeon::MeshHeight, zAlpha), Vertex::Left, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha + Dungeon::MeshScale, Dungeon::MeshHeight, zBeta), Vertex::Left, glm::vec2(1.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha + Dungeon::MeshScale, Dungeon::MeshHeight, zAlpha), Vertex::Left, glm::vec2(0.0f, 1.0f)));
				}

				if (Decoder::Cells[row][column + 1] == CLOSED_TILE)
				{
					//Back wall face of exterior shell of dungeon mesh.
					mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha + Dungeon::MeshScale), Vertex::In, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha + Dungeon::MeshScale), Vertex::In, glm::vec2(1.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta, -Dungeon::MeshHeight, zAlpha + Dungeon::MeshScale), Vertex::In, glm::vec2(1.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha, -Dungeon::MeshHeight, zAlpha + Dungeon::MeshScale), Vertex::In, glm::vec2(0.0f, 0.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xAlpha, Dungeon::MeshHeight, zAlpha + Dungeon::MeshScale), Vertex::In, glm::vec2(0.0f, 1.0f)));
					mesh.AddVertex(Vertex(glm::vec3(xBeta, Dungeon::MeshHeight, zAlpha + Dungeon::MeshScale), Vertex::In, glm::vec2(1.0f, 1.0f)));
				}
			}
		}
	}

	return true;
}

bool Dungeon::LockCamera()
{
	//TODO: Make this synchronized with other declarations.
	static const float meshScale = 1.33333333f;
	float minX = 0.0f, minY = 0.0f, clampX = 0.0f, clampY = 0.0f, distance = 0.0f;

	if (Decoder::Cells.empty())
		return Logger::SaveMessage(std::string("Error: LockCamera() - 1"));

	//TODO: only test cells within proximity to camera
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