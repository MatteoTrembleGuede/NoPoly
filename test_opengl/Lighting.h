#pragma once
#include "Vector3.h"
#include <string>

class Shader;
class Light;

class Lighting
{

public:

	Lighting(Shader* _shader);
	~Lighting();

	Vector3 sunDirection;
	Vector3 sunColor;
	Vector3 skyColor;
	Vector3 floorColor;
	Vector3 ambientColor;
	Shader* shader;

	void Reset();
	void UpdateData();
	void Save(std::string& outSave);
	void Load(std::stringstream& inSave);

};

