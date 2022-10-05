#include "Camera.hpp"
#include "Frustum.hpp"
#include "Logger.hpp"

#include "Libraries/glm/gtc/matrix_access.hpp"

Frustum::Frustum() : Planes(std::array<Plane, 6>())
{
}

Frustum::~Frustum()
{
	this->Reset();
}

Frustum::Frustum(const Frustum& frustum) : Planes(frustum.Planes)
{
}

Frustum::Frustum(Frustum&& frustum) noexcept : Planes(std::move(frustum.Planes))
{
}

Frustum& Frustum::operator=(const Frustum& frustum)
{
	this->Planes = frustum.Planes;
	return *this;
}

Frustum& Frustum::operator=(Frustum&& frustum) noexcept
{
	this->Planes = std::move(frustum.Planes);
	return *this;
}

bool Frustum::Cull(const AlignedBox& box) const
{
	glm::vec3 center = (box.Minimum + box.Maximum) * 0.5f, diagonal = box.Maximum - center;

	for (size_t index = 0; index < this->Planes.size(); ++index)
		if(glm::dot(center, this->Planes[index].Direction) + this->Planes[index].Distance + glm::dot(diagonal, glm::abs(this->Planes[index].Direction)) < 0.0f)
			return true;

	return false;
}

bool Frustum::Update()
{
	glm::mat4 matrix = Camera::Projection * Camera::View * Camera::Model;
	glm::vec4 rowX = glm::row(matrix, 0), rowY = glm::row(matrix, 1), rowZ = glm::row(matrix, 2), rowW = glm::row(matrix, 3);

	this->Planes[0] = Plane(glm::normalize(rowW + rowX));
	this->Planes[1] = Plane(glm::normalize(rowW - rowX));
	this->Planes[2] = Plane(glm::normalize(rowW + rowY));
	this->Planes[3] = Plane(glm::normalize(rowW - rowY));
	this->Planes[4] = Plane(glm::normalize(rowW + rowZ));
	this->Planes[5] = Plane(glm::normalize(rowW - rowZ));

	return true;
}

bool Frustum::Reset()
{
	for (size_t index = 0; index < Planes.size(); ++index)
		Planes[index].Reset();

	return true;
}