#ifndef Texture_H
#define Texture_H

#include <cstdint>
#include <string>

#include "glad/glad.h"

class Texture
{
public:
	GLuint Id;

	Texture();
	~Texture();
	Texture(const Texture&);
	Texture(Texture&&) noexcept;
	Texture& operator=(const Texture&);
	Texture& operator=(Texture&&) noexcept;

	bool Load(const std::string&);
	bool Use(const GLint = 0) const;
	bool Destroy();
};

#endif