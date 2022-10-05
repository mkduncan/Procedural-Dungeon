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
		std::string("out vec2 TexCoords;\n\n") +
		std::string("uniform mat4 model;\n") +
		std::string("uniform mat4 view;\n") +
		std::string("uniform mat4 projection;\n\n") +
		std::string("void main()\n") +
		std::string("{\n") +
		std::string("\tFragPos = vec3(model * vec4(aPos, 1.0));\n") +
		std::string("\tNormal = mat3(transpose(inverse(model))) * aNormal;\n") +
		std::string("\tgl_Position = projection * view * vec4(FragPos, 1.0);\n") +
		std::string("\tTexCoords = aTexCoord;\n") +
		std::string("}\n")
	),

	std::string
	(
		std::string("#version 330 core\n\n") +
		std::string("out vec4 FragColor;\n\n") +
		std::string("in vec3 FragPos;\n") +
		std::string("in vec3 Normal;\n") +
		std::string("in vec2 TexCoords;\n") +
		std::string("uniform sampler2D ourTexture;\n\n") +
		std::string("void main()\n") +
		std::string("{\n") +
		std::string("\tFragColor = texture(ourTexture, TexCoords);\n") +
		std::string("}\n")
	),

	std::string
	(
		std::string("#version 330 core\n\n") +
		std::string("out vec4 FragColor;\n\n") +
		std::string("struct Material\n") +
		std::string("{\n") +
		std::string("\tsampler2D diffuse;\n") +
		std::string("\tsampler2D specular;\n") +
		std::string("\tfloat shininess;\n") +
		std::string("};\n\n") +
		std::string("struct Light\n") +
		std::string("{\n") +
		std::string("\tvec3 position;\n") +
		std::string("\tvec3 direction;\n") +
		std::string("\tvec3 ambient;\n") +
		std::string("\tvec3 diffuse;\n") +
		std::string("\tvec3 specular;\n") +
		std::string("\tfloat cutOff;\n") +
		std::string("\tfloat outerCutOff;\n") +
		std::string("\tfloat constant;\n") +
		std::string("\tfloat linear;\n") +
		std::string("\tfloat quadratic;\n") +
		std::string("};\n\n") +
		std::string("in vec3 FragPos;\n") +
		std::string("in vec3 Normal;\n") +
		std::string("in vec2 TexCoords;\n\n") +
		std::string("uniform vec3 viewPos;\n") +
		std::string("uniform Material material;\n") +
		std::string("uniform Light light;\n\n") +
		std::string("void main()\n") +
		std::string("{\n") +
		std::string("\tvec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;\n") +
		std::string("\tvec3 norm = normalize(Normal);\n") +
		std::string("\tvec3 lightDir = normalize(light.position - FragPos);\n") +
		std::string("\tfloat diff = max(dot(norm, lightDir), 0.0);\n") +
		std::string("\tvec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;\n") +
		std::string("\tvec3 viewDir = normalize(viewPos - FragPos);\n") +
		std::string("\tvec3 reflectDir = reflect(-lightDir, norm);\n") +
		std::string("\tvec3 halfwayDir = normalize(lightDir + viewDir);\n") +
		std::string("\tfloat spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);\n") +
		std::string("\tvec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;\n") +
		std::string("\tfloat theta = dot(lightDir, normalize(-light.direction));\n") +
		std::string("\tfloat epsilon = (light.cutOff - light.outerCutOff);\n") +
		std::string("\tfloat intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n") +
		std::string("\tdiffuse *= intensity;\n") +
		std::string("\tspecular *= intensity;\n") +
		std::string("\tfloat distance = length(light.position - FragPos);\n") +
		std::string("\tfloat attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n") +
		std::string("\tambient *= attenuation;\n") +
		std::string("\tdiffuse *= attenuation;\n") +
		std::string("\tspecular *= attenuation;\n") +
		std::string("\tvec3 result = ambient + diffuse + specular;\n") +
		std::string("\tFragColor = vec4(result, 1.0);\n") +
		std::string("}\n")
	)
});