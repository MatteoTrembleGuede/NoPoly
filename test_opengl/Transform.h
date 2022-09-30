#pragma once

#include "Vector3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <list>

#define DYNAMIC_TRANSFORM_NUM 32

class Shader;

class Transform
{
public:

	Transform();
	~Transform();

	Vector3 position;
	Vector3 rotation;
	float scale;
	glm::mat4* dynMat;
	int dynMatID;

	glm::mat4 GetInvMatrix();
	glm::mat4 GetMatrix();
	std::string GetInvMatrixStr();
	std::string GetScaleStr();
	bool IsDynamic() { return (dynMat && dynMatID >= 0); }
	void SetDynamic(bool dynamic);
	
	static void Init();
	static void SendData(Shader* shader);

private:

	static bool RequestDynamic(Transform* transform);
	static void ReleaseDynamic(Transform* transform);
	static std::list<Transform*> dynTransforms;
	static glm::mat4 invMatrices[DYNAMIC_TRANSFORM_NUM];
	static float scales[DYNAMIC_TRANSFORM_NUM];
	static int matCount;

};

