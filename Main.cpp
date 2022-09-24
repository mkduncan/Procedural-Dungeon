#include "Header.hpp"

#define DEBUG_MODE true
#define ENABLE_MESSAGES true

int main(int argc, char** argv)
{
	if (DEBUG_MODE)
	{
		Decoder::ReadSourceImage("Assets/chunks.png");
		Decoder::GenerateStaticCode("StaticMap");
		Decoder::GenerateMapImage(128, 128);
		Decoder::WriteMapImage("output.png");
		Decoder::GenerateMapCode(128, 128);
		Decoder::WriteMapCode("output_map.png");
		Decoder::WriteModelCode("output");
	}

	Shader shader;
	Texture dirtTexture, rockTexture;

	if (!Window::Open())
	{
		if (ENABLE_MESSAGES)
			Logger::PrintMessages("output.txt");

		return 1;
	}

	else if (!shader.Create(StaticShader::Shaders[0], StaticShader::Shaders[1]))
	{
		Window::Close();

		if (ENABLE_MESSAGES)
			Logger::PrintMessages("output.txt");

		return 2;
	}

	else if (!dirtTexture.Load("Assets/dirt_texture.png"))
	{
		shader.Destroy();
		Window::Close();

		if (ENABLE_MESSAGES)
			Logger::PrintMessages("output.txt");

		return 3;
	}

	else if (!rockTexture.Load("Assets/rock_texture.png"))
	{
		dirtTexture.Destroy();
		shader.Destroy();
		Window::Close();

		if (ENABLE_MESSAGES)
			Logger::PrintMessages("output.txt");

		return 4;
	}

	else if (!Dungeon::GenerateMesh(128, 128))
	{
		rockTexture.Destroy();
		dirtTexture.Destroy();
		shader.Destroy();
		Window::Close();

		if (ENABLE_MESSAGES)
			Logger::PrintMessages("output.txt");

		return 5;
	}

	else if (!Camera::StartPosition())
	{
		Dungeon::DestroyMesh();
		rockTexture.Destroy();
		dirtTexture.Destroy();
		shader.Destroy();
		Window::Close();

		if (ENABLE_MESSAGES)
			Logger::PrintMessages("output.txt");

		return 6;
	}

	while (Window::Update())
	{
		Camera::Update();
		Dungeon::LockCamera();
		Camera::SetWorld(shader);

		dirtTexture.Use();
		Dungeon::RenderShellMesh(shader);

		rockTexture.Use();
		Dungeon::RenderWallMesh(shader);
	}

	Window::Close();
	rockTexture.Destroy();
	dirtTexture.Destroy();
	shader.Destroy();
	Dungeon::DestroyMesh();

	if (ENABLE_MESSAGES)
		Logger::PrintMessages("output.txt");

	return 0;
}