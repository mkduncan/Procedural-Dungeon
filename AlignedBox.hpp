#ifndef AlignedBox_H
#define AlignedBox_H

#include "Libraries/glm/glm.hpp"
#include <array>

class AlignedBox
{
public:
	glm::vec3 Minimum, Maximum;
	size_t Id;

	AlignedBox();
	~AlignedBox();
	AlignedBox(const AlignedBox&);
	AlignedBox(AlignedBox&&) noexcept;
	AlignedBox& operator=(const AlignedBox&);
	AlignedBox& operator=(AlignedBox&&) noexcept;

	std::array<glm::vec3, 8> Points() const;
	AlignedBox Merge(const AlignedBox&) const;
	AlignedBox Intersection(const AlignedBox&) const;

	bool Envelope(const glm::vec3&);
	bool Overlaps(const AlignedBox&) const;
	bool Contains(const AlignedBox&) const;

	float Width() const;
	float Height() const;
	float Depth() const;
	float SurfaceArea() const;

	bool Reset();
};

#endif