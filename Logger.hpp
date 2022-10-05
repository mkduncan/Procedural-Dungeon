#ifndef Logger_H
#define Logger_H

#define DUNGEON_LOGGER_ENABLED true

#include <queue>
#include <string>

class Logger
{
private:
	Logger() = delete;
	~Logger() = delete;
	Logger(const Logger&) = delete;
	Logger(Logger&&) noexcept = delete;
	Logger& operator=(const Logger&) = delete;
	Logger& operator=(Logger&&) noexcept = delete;

public:
	static std::queue<std::string> Logs;

	static bool SaveMessage(const std::string&, const bool = false);
	static bool PrintMessages(const std::string & = std::string());
};

#endif