#include "Vector3.h"
#include "math.h"
#include <string>

Vector3::Vector3(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

Vector3::Vector3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

Vector3::~Vector3()
{

}

Vector3 Vector3::operator+(const Vector3& vec2)
{
	return Vector3(x + vec2.x, y + vec2.y, z + vec2.z);
}

Vector3& Vector3::operator+=(const Vector3& vec2)
{
	(*this) = Vector3(x + vec2.x, y + vec2.y, z + vec2.z);
	return (*this);
}

Vector3 Vector3::operator-(const Vector3& vec2)
{
	return Vector3(x - vec2.x, y - vec2.y, z - vec2.z);
}

float Vector3::Distance(const Vector3& vec1, const Vector3& vec2)
{
	return (vec2 - vec1).Length();
}

float Vector3::Length()
{
	return sqrtf(powf(x, 2.0f) + powf(y, 2.0f) + powf(z, 2.0f));
}

Vector3& Vector3::Normalize()
{
	(*this) = (*this) / Length();
	return (*this);
}

Vector3 Vector3::operator=(const Vector3& vec2)
{
	x = vec2.x;
	y = vec2.y;
	z = vec2.z;
	return *this;
}


Vector3 Vector3::Parse(std::string& str)
{
	Vector3 vec;
	std::string tmp = str;
	for (int x = 0; x < 3; x++)
	{
		float* v = ((float*)&vec) + x;
		size_t fo = tmp.find_first_of(' ');
		fo = (fo == std::string::npos) ? tmp.size() : fo;
		std::string val = std::string(tmp.c_str(), fo);
		*v = std::stof(val);
		if (x < 2) tmp = std::string(tmp.c_str() + fo + 1);
	}
	return vec;
}

Vector3 Vector3::Cross(const Vector3& vec1, const Vector3& vec2)
{
	return Vector3(
		vec1.y * vec2.z - vec1.z * vec2.y,
		vec1.z * vec2.x - vec1.x * vec2.z,
		vec1.x * vec2.y - vec1.y * vec2.x
	);
}

float Vector3::Dot(const Vector3& vec1, const Vector3& vec2)
{
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}
