#pragma once
#include <string>
class Vector3
{
public:
	float x;
	float y;
	float z;

	Vector3(float _x, float _y, float _z);
	Vector3();
	~Vector3();
	
	Vector3 operator+(const Vector3& vec2);
	Vector3& operator+=(const Vector3& vec2);
	Vector3 operator-(const Vector3& vec2);
	Vector3 operator=(const Vector3& vec2);
	static float Distance(const Vector3& vec1, const Vector3& vec2);
	static Vector3 Parse(std::string& str);
	static Vector3 Cross(const Vector3& vec1, const Vector3& vec2);
	static float Dot(const Vector3& vec1, const Vector3& vec2);
	float Length();
	Vector3& Normalize();
};

static Vector3 operator-(const Vector3& vec1, const Vector3& vec2);
static Vector3 operator+(const Vector3& vec1, const Vector3& vec2);
static Vector3 operator/(const Vector3& vec1, const Vector3& vec2);
static Vector3 operator/(const Vector3& vec1, const float& _x);
static Vector3 operator*(const Vector3& vec1, const float& _x);
static Vector3 operator*(const float& _x, const Vector3& vec1);
static Vector3 operator*(const Vector3& vec1, const Vector3& vec2);

static Vector3 operator-(const Vector3& vec1, const Vector3& vec2)
{
	return Vector3(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
}

static Vector3 operator+(const Vector3& vec1, const Vector3& vec2)
{
	return Vector3(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}

Vector3 operator/(const Vector3& vec1, const Vector3& vec2)
{
	return Vector3(
		vec1.x / vec2.x,
		vec1.y / vec2.y,
		vec1.z / vec2.z
	);
}

Vector3 operator*(const Vector3& vec1, const Vector3& vec2)
{
	return Vector3(
		vec1.x * vec2.x,
		vec1.y * vec2.y,
		vec1.z * vec2.z
	);
}

static Vector3 operator/(const Vector3& vec1, const float& _x)
{
	return Vector3(vec1.x / _x, vec1.y / _x, vec1.z / _x);
}

static Vector3 operator*(const Vector3& vec1, const float& _x)
{
	return Vector3(vec1.x * _x, vec1.y * _x, vec1.z * _x);
}

static Vector3 operator*(const float& _x, const Vector3& vec1)
{
	return Vector3(vec1.x * _x, vec1.y * _x, vec1.z * _x);
}
