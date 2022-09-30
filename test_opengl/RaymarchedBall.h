#pragma once
#include "Vector3.h"
#include "Shader.h"

class RaymarchedBall
{
public:
	Vector3& position;
	Vector3& color;
	float& radius;
	Vector3 velocity;

	void Update(float _deltaTime);

	static RaymarchedBall* CreateBall(Vector3 position, Vector3 color, float radius);
	static void SetShader(Shader* _shader);
	static void SendData();
	static void InitData();
	~RaymarchedBall();
private:
	RaymarchedBall(Vector3 _position, Vector3 _color, float _radius);
	RaymarchedBall();
	static Vector3* positions;
	static Vector3* colors;
	static float* sizes;
	static unsigned int count;
	static Shader* shader;
};


