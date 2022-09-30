#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

class Transformable
{

public:

	virtual glm::mat4 GetLocalBase() = 0;
	virtual glm::vec3 GetPosition() = 0;
	virtual glm::vec3 GetRotation() = 0;
	virtual glm::vec3 GetScale() = 0;
	virtual void SetPosition(glm::vec3 _position) = 0;
	virtual void SetRotation(glm::vec3 _rotation) = 0;
	virtual void SetScale(glm::vec3 _scale) = 0;
	virtual bool IsDynamic() = 0;
	
};

