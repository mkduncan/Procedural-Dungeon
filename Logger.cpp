#include "Logger.hpp"

#include <fstream>
#include <iostream>
#include <string>

std::queue<std::string> Logger::Logs = std::queue<std::string>();
static bool InitializeLogger = Logger::SaveMessage(std::string());

bool Logger::SaveMessage(const std::string& message, const bool returnCode)
{
	if (DUNGEON_LOGGER_ENABLED)
	{
		if (Logs.empty())
			Logs.push("Event logger has been initialized. This should be the first message.");

		if (!message.empty())
			Logs.push(message);
	}

	return returnCode;
}

bool Logger::PrintMessages(const std::string& filePath)
{
	if (DUNGEON_LOGGER_ENABLED)
	{
		std::ofstream output;
		std::string outputBuffer;

		if (filePath.empty())
		{
			while (!Logs.empty())
			{
				std::cout << Logs.front() << "\n";
				Logs.pop();
			}

			std::cout << "Event logger has been terminated. This should be the last message.\n" << std::endl;
			return true;
		}

		output.open(filePath.c_str());

		if (!output)
			return Logger::SaveMessage("Error: Logger::PrintMessages() - 1.");

		while (!Logs.empty())
		{
			outputBuffer += Logs.front() + "\n";
			Logs.pop();
		}

		outputBuffer += "Event logger has been terminated. This should be the last message.\n";
		output << outputBuffer << std::endl;
		outputBuffer.clear();
		output.close();
	}

	return true;
}