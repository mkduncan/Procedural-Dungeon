#include "StaticShader.hpp"

std::vector<std::string> StaticShader::Shaders = std::vector<std::string>
({
	std::string
	(
		std::string("#version 330 core\n\n") +
		std::string("layout (location = 0) in vec3 aPos;\n") +
		std::string("layout (location = 1) in vec3 aNormal;\n") +
		std::string("layout (location = 2) in vec2 aTexCoord;\n\n") +
		std::string("out vec3 FragPos;\n") +
		std::string("out vec3 Normal;\n") +
		std::string("out vec2 TexCoord;\n\n") +
		std::string("uniform mat4 model;\n") +
		std::string("uniform mat4 view;\n") +
		std::string("uniform mat4 projection;\n\n") +
		std::string("void main()\n") +
		std::string("{\n") +
		std::string("\tFragPos = vec3(model * vec4(aPos, 1.0));\n") +
		std::string("\tNormal = mat3(transpose(inverse(model))) * aNormal;\n") +
		std::string("\tgl_Position = projection * view * vec4(FragPos, 1.0);\n") +
		std::string("\tTexCoord = aTexCoord;\n") +
		std::string("}\n")
	),

	std::string
	(
		std::string("#version 330 core\n\n") +
		std::string("out vec4 FragColor;\n\n") +
		std::string("in vec3 FragPos;\n") +
		std::string("in vec3 Normal;\n") +
		std::string("in vec2 TexCoord;\n") +
		std::string("uniform sampler2D ourTexture;\n\n") +
		std::string("void main()\n") +
		std::string("{\n") +
		std::string("\tFragColor = texture(ourTexture, TexCoord);\n") +
		std::string("}\n")
	)
	});