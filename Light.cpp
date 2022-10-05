#include "Light.hpp"
#include "Logger.hpp"

Light::Light() : Position(glm::vec3(0.0f)), Direction(glm::vec3(0.0f)), InnerCutoff(0.0f), OuterCutoff(0.0f), Constant(0.0f), Linear(0.0f), Quadradic(0.0f), Type(POINT_LIGHT)
{
}

Light::~Light()
{
	this->Reset();
}

Light::Light(const Light& light) : Position(light.Position), Direction(light.Direction), InnerCutoff(light.InnerCutoff), OuterCutoff(light.OuterCutoff), Constant(light.Constant), Linear(light.Linear), Quadradic(light.Quadradic), Type(light.Type)
{
}

Light::Light(Light&& light) noexcept : Position(light.Position), Direction(light.Direction), InnerCutoff(light.InnerCutoff), OuterCutoff(light.OuterCutoff), Constant(light.Constant), Linear(light.Linear), Quadradic(light.Quadradic), Type(light.Type)
{
	light.Reset();
}

Light& Light::operator=(const Light& light)
{
	this->Position = light.Position;
	this->Direction = light.Direction;
	this->InnerCutoff = light.InnerCutoff;
	this->OuterCutoff = light.OuterCutoff;
	this->Constant = light.Constant;
	this->Linear = light.Linear;
	this->Quadradic = light.Quadradic;
	this->Type = light.Type;

	return *this;
}

Light& Light::operator=(Light&& light) noexcept
{
	this->Position = light.Position;
	this->Direction = light.Direction;
	this->InnerCutoff = light.InnerCutoff;
	this->OuterCutoff = light.OuterCutoff;
	this->Constant = light.Constant;
	this->Linear = light.Linear;
	this->Quadradic = light.Quadradic;
	this->Type = light.Type;
	light.Reset();

	return *this;
}

bool Light::Load(const Shader& shader, const Material& material, const size_t lightIndex) const
{
	if (!shader.Use())
		return Logger::SaveMessage("Error: Light::Load() - 1.");

	else if (!material.Load(shader))
		return Logger::SaveMessage("Error: Light::Load() - 2.");

	if ((this->Type != DIRECTIONAL_LIGHT) && !shader.SetVector("light.position", this->Position))
		return Logger::SaveMessage("Error: Light::Load() - 3.");

	else if ((this->Type != POINT_LIGHT) && !shader.SetVector("light.direction", this->Direction))
		return Logger::SaveMessage("Error: Light::Load() - 4.");

	else if (!shader.SetVector("light.ambient", material.Ambient))
		return Logger::SaveMessage("Error: Light::Load() - 5.");

	else if (!shader.SetVector("light.diffuse", material.Diffuse))
		return Logger::SaveMessage("Error: Light::Load() - 6.");

	else if (!shader.SetVector("light.specular", material.Specular))
		return Logger::SaveMessage("Error: Light::Load() - 7.");

	else if ((this->Type == SPOT_LIGHT) && !shader.SetFloat("light.cutOff", glm::cos(glm::radians(this->InnerCutoff))))
		return Logger::SaveMessage("Error: Light::Load() - 8.");

	else if ((this->Type != SPOT_LIGHT) && !shader.SetFloat("light.outerCutOff", glm::cos(glm::radians(this->OuterCutoff))))
		return Logger::SaveMessage("Error: Light::Load() - 9.");

	else if ((this->Type != DIRECTIONAL_LIGHT) && !shader.SetFloat("light.constant", this->Constant))
		return Logger::SaveMessage("Error: Light::Load() - 10.");

	else if ((this->Type != DIRECTIONAL_LIGHT) && !shader.SetFloat("light.linear", this->Linear))
		return Logger::SaveMessage("Error: Light::Load() - 11.");

	else if ((this->Type != DIRECTIONAL_LIGHT) && !shader.SetFloat("light.quadratic", this->Quadradic))
		return Logger::SaveMessage("Error: Light::Load() - 12.");

	return true;
}

bool Light::Reset()
{
	this->Position = glm::vec3(0.0f);
	this->Direction = glm::vec3(0.0f);
	this->InnerCutoff = 0.0f;
	this->OuterCutoff = 0.0f;
	this->Constant = 0.0f;
	this->Linear = 0.0f;
	this->Quadradic = 0.0f;
	this->Type = POINT_LIGHT;

	return true;
}