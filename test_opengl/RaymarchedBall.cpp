#include "RaymarchedBall.h"

Vector3* RaymarchedBall::positions;
Vector3* RaymarchedBall::colors;
float* RaymarchedBall::sizes;
unsigned int RaymarchedBall::count;
Shader* RaymarchedBall::shader;

RaymarchedBall::RaymarchedBall() : 
	position(positions[count]), color(colors[count]), radius(sizes[count])
{
	++count;
	position = Vector3(0, 0, 0);
	color = Vector3(0, 0, 0);
	radius = 1.0f;
	velocity = Vector3(0, 0, 0);
}

RaymarchedBall::RaymarchedBall(Vector3 _position, Vector3 _color, float _radius) :
	position(positions[count]), color(colors[count]), radius(sizes[count])
{
	++count;
	position = _position;
	color = _color;
	radius = _radius;
	velocity = Vector3(0, 0, 0);
}

void RaymarchedBall::Update(float _deltaTime)
{
	velocity += Vector3(0.0f, -1.0f, 0.0f) * 9.81f * _deltaTime;
	position += velocity * _deltaTime;

	if (position.y - radius < -0.5f)
	{
		position.y = (-0.5f + radius) + (-0.5f - (position.y - radius));
		velocity.y = -velocity.y;
	}
}

RaymarchedBall* RaymarchedBall::CreateBall(Vector3 position, Vector3 color, float radius)
{
	if (count < 100)
	{
		return new RaymarchedBall(position, color, radius);
	}
	else
	{
		return nullptr;
	}
}

void RaymarchedBall::SetShader(Shader* _shader)
{
	shader = _shader;
}

void RaymarchedBall::SendData()
{
	if (shader == nullptr) return;
	shader->setVectorArray("positions", count, positions);
	shader->setVectorArray("colors", count, colors);
	shader->setFloatArray("sizes", count, sizes);
	shader->setInt("count", count);
}

void RaymarchedBall::InitData()
{
	positions = new Vector3[100]();
	colors = new Vector3[100]();
	sizes = new float[100]();
	count = 0;
}

RaymarchedBall::~RaymarchedBall()
{
	count--;
	position = positions[count];
	color = colors[count];
	radius = sizes[count];
}
