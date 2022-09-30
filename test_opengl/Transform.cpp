#include "Transform.h"
#include "Shader.h"

#define STR(a) std::to_string(a)

glm::mat4 Transform::invMatrices[DYNAMIC_TRANSFORM_NUM];
float Transform::scales[DYNAMIC_TRANSFORM_NUM];
std::list<Transform*> Transform::dynTransforms;
int Transform::matCount;

Transform::Transform()
{
	position = Vector3(0, 0, 0);
	rotation = Vector3(0, 0, 0);
	scale = 1;
	dynMat = nullptr;
	dynMatID = -1;
}

Transform::~Transform()
{
	if (IsDynamic()) ReleaseDynamic(this);
}

glm::mat4 Transform::GetInvMatrix()
{
	glm::mat4 translationMat = glm::mat4(1.0f);
	glm::mat4 rotationMat = glm::mat4(1.0f);

	rotationMat = glm::rotate(rotationMat, rotation.z, glm::vec3(0, 0, 1));
	rotationMat = glm::rotate(rotationMat, rotation.y, glm::vec3(0, 1, 0));
	rotationMat = glm::rotate(rotationMat, rotation.x, glm::vec3(1, 0, 0));

	translationMat = glm::translate(translationMat, glm::vec3(position.x, position.y, position.z));

	return glm::inverse(translationMat * rotationMat);
}

glm::mat4 Transform::GetMatrix()
{
	glm::mat4 translationMat = glm::mat4(1.0f);
	glm::mat4 rotationMat = glm::mat4(1.0f);
	glm::mat4 scaleMat = glm::mat4(1.0f);

	scaleMat = glm::scale(scaleMat, glm::vec3(scale, scale, scale));

	rotationMat = glm::rotate(rotationMat, rotation.z, glm::vec3(0, 0, 1));
	rotationMat = glm::rotate(rotationMat, rotation.y, glm::vec3(0, 1, 0));
	rotationMat = glm::rotate(rotationMat, rotation.x, glm::vec3(1, 0, 0));

	translationMat = glm::translate(translationMat, -glm::vec3(position.x, position.y, position.z));

	return (translationMat * rotationMat * scaleMat);
}

std::string Transform::GetInvMatrixStr()
{
	if (IsDynamic())
	{
		return "dynTransforms[" + STR(dynMatID) + "]";
	}
	else
	{
		glm::mat4 invTransformMat = GetInvMatrix();

		return "mat4("
			+ STR(invTransformMat[0][0]) + "," + STR(invTransformMat[0][1]) + "," + STR(invTransformMat[0][2]) + "," + STR(invTransformMat[0][3]) + ",\n"
			+ STR(invTransformMat[1][0]) + "," + STR(invTransformMat[1][1]) + "," + STR(invTransformMat[1][2]) + "," + STR(invTransformMat[1][3]) + ",\n"
			+ STR(invTransformMat[2][0]) + "," + STR(invTransformMat[2][1]) + "," + STR(invTransformMat[2][2]) + "," + STR(invTransformMat[2][3]) + ",\n"
			+ STR(invTransformMat[3][0]) + "," + STR(invTransformMat[3][1]) + "," + STR(invTransformMat[3][2]) + "," + STR(invTransformMat[3][3]) + ")";
	}
}

std::string Transform::GetScaleStr()
{
	if (IsDynamic())
	{
		return "dynScales[" + STR(dynMatID) + "]";
	}
	else
	{
		return STR(scale);
	}
}

bool Transform::RequestDynamic(Transform* transform)
{
	if (!transform->IsDynamic() && matCount < DYNAMIC_TRANSFORM_NUM)
	{
		transform->dynMat = &invMatrices[matCount];
		transform->dynMatID = matCount;
		dynTransforms.push_back(transform);
		matCount++;
		return true;
	}
	else
	{
		return false;
	}
}

void Transform::ReleaseDynamic(Transform* transform)
{
	if (transform->IsDynamic())
	{
		if (dynTransforms.size() > 1)
		{
			Transform* lastTransform = nullptr;

			for (auto tmpTransform : dynTransforms)
			{
				if (!lastTransform || tmpTransform->dynMatID > lastTransform->dynMatID)
				{
					lastTransform = tmpTransform;
				}
			}

			if (lastTransform)
			{
				lastTransform->dynMat = transform->dynMat;
				lastTransform->dynMatID = transform->dynMatID;
			}
		}

		transform->dynMat = nullptr;
		transform->dynMatID = -1;
		dynTransforms.remove(transform);
		matCount--;
	}
}

void Transform::SetDynamic(bool dynamic)
{
	if (IsDynamic() == dynamic) return;

	if (dynamic)
	{
		RequestDynamic(this);
	}
	else
	{
		ReleaseDynamic(this);
	}
}

void Transform::Init()
{
	matCount = 0;
}

void Transform::SendData(Shader* shader)
{
	for (auto transform : dynTransforms)
	{
		*transform->dynMat = transform->GetInvMatrix();
		scales[transform->dynMatID] = transform->scale;
	}

	shader->setMat4Array("dynTransforms", DYNAMIC_TRANSFORM_NUM, invMatrices);
	shader->setFloatArray("dynScales", DYNAMIC_TRANSFORM_NUM, scales);
}

