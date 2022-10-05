#include "Logger.hpp"
#include "Shader.hpp"

#include <fstream>
#include <sstream>

Shader::Shader() : Id(0)
{
}

Shader::~Shader()
{
	this->Destroy();
}

Shader::Shader(const Shader& shader) : Id(shader.Id)
{
}

Shader::Shader(Shader&& shader) noexcept : Id(shader.Id)
{
	shader.Id = 0;
}

Shader& Shader::operator=(const Shader& shader)
{
	if (this->Id != shader.Id)
	{
		this->Destroy();
		this->Id = shader.Id;
	}

	return *this;
}

Shader& Shader::operator=(Shader&& shader) noexcept
{
	if (this->Id != shader.Id)
	{
		this->Destroy();
		this->Id = shader.Id;
	}

	shader.Id = 0;
	return *this;
}

bool Shader::Create(const std::string &vertexCode, const std::string &fragmentCode)
{
	const char* vertexSource = vertexCode.c_str(), * fragmentSource = fragmentCode.c_str();
	char compileStatus[1024] = { 0 };
	GLuint vertexId = 0, fragmentId = 0;
	GLint status = 0;

	if (this->Id != 0)
		return Logger::SaveMessage("Error: Shader::Create() - 1.");

	else if(vertexCode.empty())
		return Logger::SaveMessage("Error: Shader::Create() - 2.");

	else if(fragmentCode.empty())
		return Logger::SaveMessage("Error: Shader::Create() - 3.");

	vertexId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexId, 1, &vertexSource, nullptr);
	glCompileShader(vertexId);
	glGetShaderiv(vertexId, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderInfoLog(vertexId, 1024, nullptr, &compileStatus[0]);
		glDeleteShader(vertexId);

		Logger::SaveMessage("Error: Shader::Create() - 4.");
		return Logger::SaveMessage(std::string("\t") + &compileStatus[0]);
	}

	fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentId, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentId);
	glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderInfoLog(fragmentId, 1024, nullptr, &compileStatus[0]);
		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);

		Logger::SaveMessage("Error: Shader::Create() - 5.");
		return Logger::SaveMessage(std::string("\t") + &compileStatus[0]);
	}

	this->Id = glCreateProgram();
	glAttachShader(this->Id, vertexId);
	glAttachShader(this->Id, fragmentId);
	glLinkProgram(this->Id);
	glGetProgramiv(this->Id, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetProgramInfoLog(this->Id, 1024, nullptr, &compileStatus[0]);
		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);
		glDeleteProgram(this->Id);

		Logger::SaveMessage("Error: Shader::Create() - 6.");
		return Logger::SaveMessage(std::string("\t") + &compileStatus[0]);
	}

	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);

	return true;
}

bool Shader::Load(const std::string &vertexFile, const std::string &fragmentFile)
{
	std::ifstream inputFile;
	std::stringstream vertexBuffer, fragmentBuffer;

	inputFile.open(vertexFile);

	if (!inputFile)
		return Logger::SaveMessage("Error: Shader::Load() - 1.");

	vertexBuffer << inputFile.rdbuf();
	inputFile.close();
	inputFile.open(fragmentFile);

	if (!inputFile)
		return Logger::SaveMessage("Error: Shader::Load() - 2.");

	fragmentBuffer << inputFile.rdbuf();
	inputFile.close();

	if (!this->Create(vertexBuffer.str(), fragmentBuffer.str()))
		return Logger::SaveMessage("Error: Shader::Load() - 3.");

	vertexBuffer = std::stringstream();
	fragmentBuffer = std::stringstream();

	return true;
}

bool Shader::Use() const
{
	if (this->Id == 0)
	{
		glUseProgram(0);
		return Logger::SaveMessage("Error: Shader::Use() - 1.");
	}

	glUseProgram(this->Id);
	return true;
}

bool Shader::Destroy()
{
	if (this->Id != 0)
	{
		glDeleteProgram(this->Id);
		this->Id = 0;
	}

	return true;
}

bool Shader::SetBoolean(const std::string &key, const bool value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetBoolean() - 1.");

	glUniform1i(glGetUniformLocation(this->Id, key.c_str()), static_cast<GLint>(value));
	return true;
}

bool Shader::SetInteger(const std::string &key, const GLint value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetInteger() - 1.");

	glUniform1i(glGetUniformLocation(this->Id, key.c_str()), value);
	return true;
}

bool Shader::SetFloat(const std::string &key, const float value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetFloat() - 1.");

	glUniform1f(glGetUniformLocation(this->Id, key.c_str()), value);
	return true;
}

bool Shader::SetVector(const std::string &key, const glm::vec2 &value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetVector*2() - 1.");

	glUniform2fv(glGetUniformLocation(this->Id, key.c_str()), 1, &value[0]);
	return true;
}

bool Shader::SetVector(const std::string &key, const glm::vec3 &value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetVector*3() - 1.");

	glUniform3fv(glGetUniformLocation(this->Id, key.c_str()), 1, &value[0]);
	return true;
}

bool Shader::SetVector(const std::string &key, const glm::vec4 &value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetVector*4() - 1.");

	glUniform4fv(glGetUniformLocation(this->Id, key.c_str()), 1, &value[0]);
	return true;
}

bool Shader::SetMatrix(const std::string &key, const glm::mat2 &value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetMatrix*2() - 1.");

	glUniformMatrix2fv(glGetUniformLocation(this->Id, key.c_str()), 1, GL_FALSE, &value[0][0]);
	return true;
}

bool Shader::SetMatrix(const std::string &key, const glm::mat3 &value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetMatrix*3() - 1.");

	glUniformMatrix3fv(glGetUniformLocation(this->Id, key.c_str()), 1, GL_FALSE, &value[0][0]);
	return true;
}

bool Shader::SetMatrix(const std::string &key, const glm::mat4 &value) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Shader::SetMatrix*4() - 1.");

	glUniformMatrix4fv(glGetUniformLocation(this->Id, key.c_str()), 1, GL_FALSE, &value[0][0]);
	return true;
}