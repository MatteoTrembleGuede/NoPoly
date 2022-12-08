#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

class BoundingVolume
{

public:

	enum Type
	{
		Box,
		Sphere,
		Ellipsoid,
		Count
	};

	union Size
	{
		float s;
		glm::vec3 b;
	};

	Type type;
	Size size;
	glm::vec3 offset;
	glm::vec3 color;

	BoundingVolume();

};

