#include "Lighting.h"
#include "Shader.h"
#include "Light.h"

#define STR(a) std::to_string(a)

Lighting::Lighting(Shader* _shader)
{
	shader = _shader;
	Reset();
}

Lighting::~Lighting()
{

}

void Lighting::Reset()
{
	sunDirection = Vector3(0.0f, -1.0f, 1.0f);
	sunColor = Vector3(1.0f, 0.9f, 0.7f);
	skyColor = Vector3(0.3f, 0.6f, 1.0f) * 0.5f;
	ambientColor = Vector3(0.05f, 0.05f, 0.05f);
	floorColor = Vector3(0.3f, 0.9f, 0.2f) / 10.0f;
}

void Lighting::UpdateData()
{
	if (sunDirection.x != 0.0f || sunDirection.y != 0.0f || sunDirection.z != 0.0f)
	{
		sunDirection.Normalize();
	}
	shader->setVector("sunDirection", sunDirection);
	shader->setVector("sunColor", sunColor);
	shader->setVector("skyColor", skyColor);
	shader->setVector("ambientColor", ambientColor);
	shader->setVector("floorColor", floorColor);
	
	Light::SendData(shader);
}

void Lighting::Save(std::string& outSave)
{
	outSave += "sunDirection " + STR(sunDirection.x) + " " + STR(sunDirection.y) + " " + STR(sunDirection.z) + "\n";
	outSave += "sunColor " + STR(sunColor.x) + " " + STR(sunColor.y) + " " + STR(sunColor.z) + "\n";
	outSave += "skyColor " + STR(skyColor.x) + " " + STR(skyColor.y) + " " + STR(skyColor.z) + "\n";
	outSave += "floorColor " + STR(floorColor.x) + " " + STR(floorColor.y) + " " + STR(floorColor.z) + "\n";
	outSave += "ambientColor " + STR(ambientColor.x) + " " + STR(ambientColor.y) + " " + STR(ambientColor.z) + "\n";
	outSave += "lightCount " + STR(Light::lights.size()) + "\n";

	for (auto it = Light::lights.begin(); it != Light::lights.end(); ++it)
	{
		(*it)->Save(outSave);
	}
}

void Lighting::Load(std::stringstream& inSave)
{
	char buf[512];
	int lightCount = 0;
	for (int i = 0; i < 6; i++)
	{
		inSave.getline(buf, 512, '\n');
		if (inSave.eof()) return;
		std::string line = buf;
		std::string word = std::string(line.c_str(), line.find_first_of(' '));
		std::string value = std::string(line.c_str() + line.find_first_of(' ') + 1);

		if (word == "sunDirection")
		{
			sunDirection = Vector3::Parse(value);
		}
		else if (word == "sunColor")
		{
			sunColor = Vector3::Parse(value);
		}
		else if (word == "skyColor")
		{
			skyColor = Vector3::Parse(value);
		}
		else if (word == "floorColor")
		{
			floorColor = Vector3::Parse(value);
		}
		else if (word == "ambientColor")
		{
			ambientColor = Vector3::Parse(value);
		}
		else if (word == "lightCount")
		{
			lightCount = std::stoi(value);
		}
	}

	for (int i = 0; i < lightCount; ++i)
	{
		Light* light = Light::Create();
		if (light)
		{
			light->Load(inSave);
		}
		else
		{
			break;
		}
	}
}
