#include "Decoder.hpp"
#include "Logger.hpp"
#include "Texture.hpp"

Texture::Texture() : Id(0)
{
}

Texture::~Texture()
{
	this->Destroy();
}

Texture::Texture(const Texture& texture) : Id(texture.Id)
{
}

Texture::Texture(Texture&& texture) noexcept : Id(texture.Id)
{
	texture.Id = 0;
}

Texture& Texture::operator=(const Texture& texture)
{
	if (this->Id != texture.Id)
	{
		this->Destroy();
		this->Id = texture.Id;
	}

	return *this;
}

Texture& Texture::operator=(Texture&& texture) noexcept
{
	if (this->Id != texture.Id)
	{
		this->Destroy();
		this->Id = texture.Id;
	}

	texture.Id = 0;
	return *this;
}

bool Texture::Load(const std::string &imagePath)
{
	if (this->Id != 0)
		return Logger::SaveMessage("Error: Texture::Load() - 1.");

	else if(!Decoder::ReadSourceImage(imagePath))
		return Logger::SaveMessage("Error: Texture::Load() - 2.");

	glGenTextures(1, &this->Id);
	glBindTexture(GL_TEXTURE_2D, this->Id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(Decoder::Width), static_cast<GLsizei>(Decoder::Height), 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<const void*>(&Decoder::Buffer[0]));
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

bool Texture::Use(const GLint activeID) const
{
	if (this->Id == 0)
		return Logger::SaveMessage("Error: Texture::Use() - 1.");

	if(activeID < 32)
		glActiveTexture(GL_TEXTURE0 + activeID);

	else
		return Logger::SaveMessage("Error: Texture::Use() - 2.");

	glBindTexture(GL_TEXTURE_2D, this->Id);
	return true;
}

bool Texture::Destroy()
{
	if (this->Id != 0)
	{
		glDeleteTextures(1, &this->Id);
		this->Id = 0;
	}

	return true;
}