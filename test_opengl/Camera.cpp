#include "Camera.h"
#include "Shader.h"
#include "ViewportManager.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Input/Input.h"

float Camera::speed;
float Camera::fov;

void Camera::RebuildDirection()
{
	right = Vector3::Cross(forward, Vector3(0, 1, 0)).Normalize();
	up = Vector3::Cross(right, forward).Normalize();
}

void Camera::RotateCameraX(float rot, float lastRot)
{
	xRotInput += rot;
}

void Camera::RotateCameraY(float rot, float lastRot)
{
	yRotInput += rot;
}

void Camera::SetMoveCamera()
{
	Input::MousePos mp = Input::GetMousePosition();
	if (ViewportManager::IsQuadContaining(ImVec2(mp.x, mp.y)))
	{
		Input::SetCursorVisible(false);
		Input::GetGlobalInput(0).BindAxis("MoveX", this, &Camera::MoveX);
		Input::GetGlobalInput(0).BindAxis("MoveY", this, &Camera::MoveY);
		Input::GetGlobalInput(0).BindAxis("MoveZ", this, &Camera::MoveZ);
		Input::GetGlobalInput(0).BindAxis("ModifySpeed", this, &Camera::ModifySpeed);
		Input::GetGlobalInput(1).BindAxis("MoveMouseX", this, &Camera::RotateCameraX);
		Input::GetGlobalInput(1).BindAxis("MoveMouseY", this, &Camera::RotateCameraY);
		Input::GetGlobalInput(0).BindAction("ResetCamera", Input::Mode::Press, this, &Camera::Reset);
	}
}

void Camera::UnsetMoveCamera()
{
	Input::SetCursorVisible(true);
	Input::GetGlobalInput(0).UnbindAxis("MoveX", this, &Camera::MoveX);
	Input::GetGlobalInput(0).UnbindAxis("MoveY", this, &Camera::MoveY);
	Input::GetGlobalInput(0).UnbindAxis("MoveZ", this, &Camera::MoveZ);
	Input::GetGlobalInput(0).UnbindAxis("ModifySpeed", this, &Camera::ModifySpeed);
	Input::GetGlobalInput(1).UnbindAxis("MoveMouseX", this, &Camera::RotateCameraX);
	Input::GetGlobalInput(1).UnbindAxis("MoveMouseY", this, &Camera::RotateCameraY);
	Input::GetGlobalInput(0).UnbindAction("ResetCamera", Input::Mode::Press, this, &Camera::Reset);
}

void Camera::Reset()
{
	SetPosition(Vector3());
	LookAt(Vector3(0, 0, 1));
	speed = 1;
}

void Camera::ModifySpeed(float dummy, float delta)
{
	speed += delta;
	if (speed < 1) speed = 1;
}

Camera::Camera()
{
	speed = 1.0f;
	position = Vector3(0, 0, 0);
	forward = Vector3(0, 0, 1);
	up = Vector3(0, 1, 0);
	right = Vector3(-1, 0, 0);
	fov = 1.571f;
	/*Input::GetGlobalInput(0).AddAxis("RotateCameraX", Input::Key(Input::MouseAxis::Horizontal));
	Input::GetGlobalInput(0).AddAxis("RotateCameraY", Input::Key(Input::MouseAxis::Vertical));*/
	Input::GetGlobalInput(0).AddAxis("MoveX", Input::Key(Input::KeyCode::D), Input::Key(Input::KeyCode::A));
	Input::GetGlobalInput(0).AddAxis("MoveY", Input::Key(Input::KeyCode::E), Input::Key(Input::KeyCode::Q));
	Input::GetGlobalInput(0).AddAxis("MoveZ", Input::Key(Input::KeyCode::W), Input::Key(Input::KeyCode::S));
	Input::GetGlobalInput(0).AddAxis("ModifySpeed", Input::Key(Input::KeyCode::MOUSEWUP), Input::Key(Input::KeyCode::MOUSEWDOWN));
	Input::GetGlobalInput(0).AddAction("SetMoveCamera", Input::Key(Input::KeyCode::MOUSE1));
	Input::GetGlobalInput(0).AddAction("ResetCamera", Input::Key(Input::KeyCode::BACKSPACE));
	Input::GetGlobalInput(0).BindAction("SetMoveCamera", Input::Mode::Press, this, &Camera::SetMoveCamera);
	Input::GetGlobalInput(0).BindAction("SetMoveCamera", Input::Mode::Release, this, &Camera::UnsetMoveCamera);
}

Camera::~Camera()
{

}

void Camera::MoveX(float current, float last)
{
	input.x += current;
}

void Camera::MoveY(float current, float last)
{
	input.y += current;
}

void Camera::MoveZ(float current, float last)
{
	input.z += current;
}

void Camera::Move(float _deltaTime)
{
	float alpha = std::max(std::min(1.0f, _deltaTime * 30.0f), 0.0f);
	float xrot = alpha * xRotInput;
	float yrot = alpha * yRotInput;
	xRotInput = (1 - alpha) * xRotInput;
	yRotInput = (1 - alpha) * yRotInput;
	LookAt(GetPosition() + GetForward() + (xrot * GetRight()) / 100.0f);
	LookAt(GetPosition() + GetForward() - (yrot * GetUp()) / 100.0f);
	position += (input.x * right + input.y * up + input.z * forward) * _deltaTime * speed;
	input = Vector3(0, 0, 0);
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

