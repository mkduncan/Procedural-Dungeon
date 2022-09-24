#ifndef Shader_H
#define Shader_H

#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"

class Shader
{
private:
	GLuint Id;

public:
	Shader();
	~Shader();
	Shader(const Shader&);
	Shader(Shader&&) noexcept = default;
	Shader& operator=(const Shader&);
	Shader& operator=(Shader&&) noexcept = default;

	bool Create(const std::string&, const std::string&);
	bool Use() const;
	bool Destroy();

	bool SetBoolean(const std::string&, const bool) const;
	bool SetInteger(const std::string&, const GLint) const;
	bool SetFloat(const std::string&, const float) const;
	bool SetVector(const std::string&, const glm::vec2&) const;
	bool SetVector(const std::string&, const glm::vec3&) const;
	bool SetVector(const std::string&, const glm::vec4&) const;
	bool SetMatrix(const std::string&, const glm::mat2&) const;
	bool SetMatrix(const std::string&, const glm::mat3&) const;
	bool SetMatrix(const std::string&, const glm::mat4&) const;
};

#endif