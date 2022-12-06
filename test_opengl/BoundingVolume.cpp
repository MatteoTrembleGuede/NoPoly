#include "BoundingVolume.h"

BoundingVolume::BoundingVolume()
{
	type = Box;
	size.b = glm::vec3(1.0f, 1.0f, 1.0f);
}
