#ifndef StaticShader_H
#define StaticShader_H

#include <cstdint>
#include <string>
#include <vector>

class StaticShader
{
private:
	StaticShader() = delete;
	~StaticShader() = delete;
	StaticShader(const StaticShader&) = delete;
	StaticShader(StaticShader&&) noexcept = delete;
	StaticShader& operator=(const StaticShader&) = delete;
	StaticShader& operator=(StaticShader&&) noexcept = delete;

public:
	static std::vector<std::string> Shaders;
};

#endif