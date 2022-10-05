#include "Header.hpp"

int main(int argc, char** argv)
{
	Scene primaryScene;
	uint16_t sceneWidth = 256, sceneHeight = 256;

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

	if (!Window::Open())
	{
		Logger::PrintMessages("output.txt");
		return 1;
	}

	else if (!primaryScene.GenerateDungeon(sceneWidth, sceneHeight))
	{
		Logger::PrintMessages("output.txt");
		return 2;
	}

	while (Window::Update())
	{
		if (!Camera::Update(false, false))
		{
			Window::Close();
			Logger::PrintMessages("output.txt");

			return 3;
		}

		else if (!primaryScene.Update())
		{
			Window::Close();
			Logger::PrintMessages("output.txt");

			return 4;
		}
		
		if (!primaryScene.Render())
		{
			Window::Close();
			Logger::PrintMessages("output.txt");

			return 5;
		}
	}

	if (!Window::Close())
	{
		primaryScene.Destroy();
		Logger::PrintMessages("output.txt");

		return 6;
	}

	else if (!primaryScene.Destroy())
	{
		Logger::PrintMessages("output.txt");
		return 7;
	}

	Logger::PrintMessages("output.txt");
	return 0;
}