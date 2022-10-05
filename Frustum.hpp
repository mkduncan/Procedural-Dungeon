#ifndef Frustum_H
#define Frustum_H

#include "AlignedBox.hpp"
#include "Plane.hpp"
#include <array>

class Frustum
{
public:
	std::array<Plane, 6> Planes;

	Frustum();
	~Frustum();
	Frustum(const Frustum&);
	Frustum(Frustum&&) noexcept;
	Frustum& operator=(const Frustum&);
	Frustum& operator=(Frustum&&) noexcept;

	bool Cull(const AlignedBox&) const;
	bool Update();
	bool Reset();
};

#endif