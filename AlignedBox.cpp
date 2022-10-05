#include "AlignedBox.hpp"
#include <limits>

AlignedBox::AlignedBox() : Minimum(glm::vec3(std::numeric_limits<float>::max())), Maximum(glm::vec3(-std::numeric_limits<float>::max())), Id(0)
{
}

AlignedBox::~AlignedBox()
{
	this->Reset();
}

AlignedBox::AlignedBox(const AlignedBox& box) : Minimum(box.Minimum), Maximum(box.Maximum), Id(box.Id)
{
}

AlignedBox::AlignedBox(AlignedBox&& box) noexcept : Minimum(box.Minimum), Maximum(box.Maximum), Id(box.Id)
{
	box.Reset();
}

AlignedBox& AlignedBox::operator=(const AlignedBox& box)
{
	this->Minimum = box.Minimum;
	this->Maximum = box.Maximum;
	this->Id = box.Id;

	return *this;
}

AlignedBox& AlignedBox::operator=(AlignedBox&& box) noexcept
{
	this->Minimum = box.Minimum;
	this->Maximum = box.Maximum;
	this->Id = box.Id;
	box.Reset();

	return *this;
}

std::array<glm::vec3, 8> AlignedBox::Points() const
{
	return std::array<glm::vec3, 8>
	({
		glm::vec3(this->Minimum.x, this->Minimum.y, this->Minimum.z),
		glm::vec3(this->Minimum.x, this->Maximum.y, this->Minimum.z),
		glm::vec3(this->Minimum.x, this->Minimum.y, this->Maximum.z),
		glm::vec3(this->Minimum.x, this->Maximum.y, this->Maximum.z),
		glm::vec3(this->Maximum.x, this->Minimum.y, this->Minimum.z),
		glm::vec3(this->Maximum.x, this->Maximum.y, this->Minimum.z),
		glm::vec3(this->Maximum.x, this->Minimum.y, this->Maximum.z),
		glm::vec3(this->Maximum.x, this->Maximum.y, this->Maximum.z)
	});
}


AlignedBox AlignedBox::Merge(const AlignedBox &box) const
{
	AlignedBox result;

	result.Minimum = glm::vec3(std::min(this->Minimum.x, box.Minimum.x), std::min(this->Minimum.y, box.Minimum.y), std::min(this->Minimum.z, box.Minimum.z));
	result.Maximum = glm::vec3(std::max(this->Maximum.x, box.Maximum.x), std::max(this->Maximum.y, box.Maximum.y), std::max(this->Maximum.z, box.Maximum.z));

	return result;
}

AlignedBox AlignedBox::Intersection(const AlignedBox &box) const
{
	AlignedBox result;

	result.Minimum = glm::vec3(std::max(this->Minimum.x, box.Minimum.x), std::max(this->Minimum.y, box.Minimum.y), std::max(this->Minimum.z, box.Minimum.z));
	result.Maximum = glm::vec3(std::min(this->Maximum.x, box.Maximum.x), std::min(this->Maximum.y, box.Maximum.y), std::min(this->Maximum.z, box.Maximum.z));

	return result;
}

bool AlignedBox::Overlaps(const AlignedBox& box) const
{
	return this->Maximum.x > box.Minimum.x && this->Minimum.x < box.Maximum.x && this->Maximum.y > box.Minimum.y && this->Minimum.y < box.Maximum.y && this->Maximum.z > box.Minimum.z && this->Minimum.z < box.Maximum.z;
}

bool AlignedBox::Contains(const AlignedBox& box) const
{
	return box.Minimum.x >= this->Minimum.x && box.Maximum.x <= this->Maximum.x && box.Minimum.y >= this->Minimum.y && box.Maximum.y <= this->Maximum.y && box.Minimum.z >= this->Minimum.z && box.Maximum.z <= this->Maximum.z;
}

float AlignedBox::Width() const
{
	return this->Maximum.x - this->Minimum.x;
}

float AlignedBox::Height() const
{
	return this->Maximum.y - this->Minimum.y;
}

float AlignedBox::Depth() const
{
	return this->Maximum.z - this->Minimum.z;
}

float AlignedBox::SurfaceArea() const
{
	return 2.0f * (this->Width() * this->Height() + this->Width() * this->Depth() + this->Height() * this->Depth());
}

bool AlignedBox::Envelope(const glm::vec3& point)
{
	this->Minimum = glm::min(this->Minimum, point);
	this->Maximum = glm::max(this->Maximum, point);

	return true;
}

bool AlignedBox::Reset()
{
	this->Minimum = glm::vec3(std::numeric_limits<float>::max());
	this->Maximum = glm::vec3(-std::numeric_limits<float>::max());

	return true;
}