#include "Camera.h"
#include "Shader.h"
#include "ViewportManager.h"
#include "ImGuizmo/ImGuizmo.h"
#include <GLFW/glfw3.h>

float Camera::speed;
float Camera::fov;

void Camera::RebuildDirection()
{
	right = Vector3::Cross(forward, Vector3(0, 1, 0)).Normalize();
	up = Vector3::Cross(right, forward).Normalize();
}

Camera::Camera()
{
	speed = 1.0f;
	position = Vector3(0, 0, 0);
	forward = Vector3(0, 0, 1);
	up = Vector3(0, 1, 0);
	right = Vector3(-1, 0, 0);
	fov = 1.571f;
}

Camera::~Camera()
{

}

void Camera::Move(Vector3 _input, float _deltaTime)
{
	position += (_input.x * right + _input.y * up + _input.z * forward) * _deltaTime * speed;
}

void Camera::SetPosition(Vector3 _newPosition)
{
	position = _newPosition;
}

Vector3 Camera::GetPosition()
{
	return position;
}

Vector3 Camera::GetForward()
{
	return forward;
}

Vector3 Camera::GetUp()
{
	return up;
}

Vector3 Camera::GetRight()
{
	return right;
}

void Camera::LookAt(Vector3 _targetPosition)
{
	forward = (_targetPosition - position).Normalize();
	RebuildDirection();
}

void Camera::ApplyToShader(Shader* _shader)
{
	_shader->setVector("camPos", position);
	_shader->setVector("camForward", forward);
	_shader->setVector("camRight", right);
	_shader->setVector("camUp", up);
	_shader->setFloat("camAngle", fov);
}

glm::mat4 Camera::GetView()
{
	glm::vec3 posGLM = glm::vec3(position.x, position.y, position.z);
	glm::vec3 forwardGLM = glm::vec3(forward.x, forward.y, forward.z);
	glm::vec3 upGLM = glm::vec3(up.x, up.y, up.z);
	return glm::lookAt(posGLM, posGLM + forwardGLM, upGLM);
}

glm::mat4 Camera::GetInvView(glm::mat4 baseMat)
{
	glm::mat4 invBase = glm::inverse(baseMat);
	glm::vec3 posGLM = invBase * glm::vec4(-glm::vec3(position.x, position.y, position.z), 1.0f);
	glm::vec3 forwardGLM = invBase * glm::vec4(glm::vec3(forward.x, forward.y, forward.z), 0.0f);
	glm::vec3 upGLM = invBase * glm::vec4(glm::vec3(up.x, up.y, up.z), 0.0f);
	return glm::lookAt(posGLM, posGLM + forwardGLM, upGLM);
}

glm::mat4 Camera::GetProjection()
{
	ImVec2 sceneViewPos, sceneViewSize;
	ViewportManager::GetSceneViewPosAndSize(sceneViewSize, sceneViewPos);
	return glm::perspective(fov, sceneViewSize.x / sceneViewSize.y, 0.0001f, 100.0f);
}

