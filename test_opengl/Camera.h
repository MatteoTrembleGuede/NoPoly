#pragma once
#include "Vector3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <iostream>

class Shader;
struct GLFWwindow;

class Camera
{

private:

	Vector3 position;
	Vector3 forward;
	Vector3 up;
	Vector3 right;
	Vector3 input;
	static float speed;
	float xRotInput, yRotInput;

	void RebuildDirection();
	void RotateCameraX(float rot, float lastRot);
	void RotateCameraY(float rot, float lastRot);
	void SetMoveCamera();
	void UnsetMoveCamera();
	void Reset();
	void ModifySpeed(float delta, float dummy);

public:
	static float fov;
	
	Camera();
	~Camera();

	void MoveX(float current, float last);
	void MoveY(float current, float last);
	void MoveZ(float current, float last);
	void Move(float _deltaTime);
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

