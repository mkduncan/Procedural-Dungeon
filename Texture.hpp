#ifndef Texture_H
#define Texture_H

#include <cstdint>
#include <string>

#include "glad/glad.h"

class Texture
{
private:
	GLuint Id;

public:
	Texture();
	~Texture();
	Texture(const Texture&);
	Texture(Texture&&) noexcept = default;
	Texture& operator=(const Texture&);
	Texture& operator=(Texture&&) noexcept = default;

	bool Load(const std::string&);
	bool Use() const;
	bool Destroy();
};

#endif