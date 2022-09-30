#pragma once
#include "Vector3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <iostream>

class Shader;
class GLFWwindow;

class Camera
{

private:

	Vector3 position;
	Vector3 forward;
	Vector3 up;
	Vector3 right;
	static float speed;

	void RebuildDirection();

public:
	static float fov;
	
	Camera();
	~Camera();

	static void SetCameraSpeed(GLFWwindow* _window, double _x, double _y)
	{
		speed += _y;
		if (speed < 1)
		{
			speed = 1;
		}
	}

	void Move(Vector3 _input, float _deltaTime);
	void SetPosition(Vector3 _newPosition);
	Vector3 GetPosition();
	Vector3 GetForward();
	Vector3 GetUp();
	Vector3 GetRight();
	void LookAt(Vector3 _targetPosition);
	void ApplyToShader(Shader* _shader);
	glm::mat4 GetView();
	glm::mat4 GetInvView(glm::mat4 baseMat = glm::mat4(1.0f));
	glm::mat4 GetProjection();
};

