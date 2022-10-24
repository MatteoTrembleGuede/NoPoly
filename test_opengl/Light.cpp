#include "Light.h"
#include "Lighting.h"
#include "Shader.h"
#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"
#include <glm/gtx/rotate_vector.hpp>

#define STR(a) std::to_string(a)

int Light::count;
int Light::types[32];
float Light::positions[96];
float Light::directions[96];
float Light::colors[128];
float Light::attribs[128];
std::list<Light*> Light::lights;
Notify Light::notifyClearLights;

Light::Light() : 
	ID(count),
	type((LightType&)types[count]),
	position((Vector3&)positions[count*3]),
	direction((Vector3&)directions[count*3]),
	color((Vector3&)colors[count*4]),
	intensity(colors[count*4 + 3]),
	radius(attribs[count*4]),
	attenuation(attribs[count*4+1]),
	aperture(attribs[count*4+2]),
	focus(attribs[count*4+3])
{
	name = "new light";
	type = LightType::directional;
	position = Vector3(0, 0, 0);
	direction = Vector3(0, 0, 1);
	color = Vector3(1, 1, 1);
	intensity = 1;
	radius = 5;
	attenuation = 2;
	aperture = 1.57f;
	focus = 1;
}

Light::~Light()
{

}

glm::mat4 Light::GetLocalBase()
{
	return glm::mat4(1.0f);
}

glm::vec3 Light::GetPosition()
{
	return -glm::vec3(position.x, position.y, position.z);
}

glm::vec3 Light::GetRotation()
{
	/*transform.rotation.x = atan2f(direction.y, glm::vec2(direction.x, direction.z).length());
	transform.rotation.y = atan2f(direction.x, direction.z);*/
	return glm::degrees(glm::vec3(transform.rotation.x, transform.rotation.y, transform.rotation.z));

	/*glm::vec3 tmpDir = (*(glm::vec3*)&direction);
	glm::mat4 rotationMat = glm::orientation(tmpDir, glm::vec3(0, 0, 1));
	glm::vec3 tmpPos = glm::vec3(0.0f);
	glm::vec3 tmpScale = glm::vec3(1.0f);
	glm::vec3 tmpRot = glm::vec3(0.0f);
	
	ImGuizmo::DecomposeMatrixToComponents((float*)&rotationMat, (float*)&tmpPos, (float*)&tmpRot, (float*)&tmpScale);
	return tmpRot;*/
}

glm::vec3 Light::GetScale()
{
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

void Light::SetPosition(glm::vec3 _position)
{
	position = (*(Vector3*)&_position) * -1.0f;
}

void Light::SetRotation(glm::vec3 _rotation)
{
	_rotation = glm::radians(_rotation);
	transform.rotation = (*(Vector3*)&_rotation);
	glm::vec3 tmpDir = glm::rotateX(glm::vec3(0, 0, 1), _rotation.x);
	tmpDir = glm::rotateY(tmpDir, _rotation.y);
	tmpDir = glm::rotateZ(tmpDir, _rotation.z);
	direction = (*(Vector3*)&tmpDir);
}

void Light::SetScale(glm::vec3 _scale)
{

}

bool Light::IsDynamic()
{
	return true;
}

void Light::Clear()
{
	for (auto it = lights.begin(); it != lights.end(); ++it)
	{
		delete (*it);
	}

	count = 0;
	lights.clear();
	notifyClearLights();
}

void Light::Save(std::string& outSave)
{
	outSave += "name " + name + "\n";
	outSave += "position " + STR(position.x) + " " + STR(position.y) + " " + STR(position.z) + "\n";
	outSave += "direction " + STR(direction.x) + " " + STR(direction.y) + " " + STR(direction.z) + "\n";
	outSave += "type " + STR((int)type) + "\n";
	outSave += "color " + STR(color.x) + " " + STR(color.y) + " " + STR(color.z) + "\n";
	outSave += "intensity " + STR(intensity) + "\n";
	outSave += "radius " + STR(radius) + "\n";
	outSave += "attenuation " + STR(attenuation) + "\n";
	outSave += "aperture " + STR(aperture) + "\n";
	outSave += "focus " + STR(focus) + "\n";
}

void Light::Load(std::stringstream& inSave)
{
	char buf[512];
	for (int i = 0; i < 10; i++)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		std::string word = std::string(line.c_str(), line.find_first_of(' '));
		std::string value = std::string(line.c_str() + line.find_first_of(' ') + 1);

		if (word == "name")
		{
			name = value;
		}
		else if (word == "position")
		{
			position = Vector3::Parse(value);
		}
		else if (word == "direction")
		{
			direction = Vector3::Parse(value);
		}
		else if (word == "type")
		{
			type = (LightType)std::stoi(value);
		}
		else if (word == "color")
		{
			color = Vector3::Parse(value);
		}
		else if (word == "intensity")
		{
			intensity = std::stof(value);
		}
		else if (word == "radius")
		{
			radius = std::stof(value);
		}
		else if (word == "attenuation")
		{
			attenuation = std::stof(value);
		}
		else if (word == "aperture")
		{
			aperture = std::stof(value);
		}
		else if (word == "focus")
		{
			focus = std::stof(value);
		}
	}
}

Light* Light::Create()
{
	if (count < 32)
	{
		Light* light = new Light();
		lights.push_back(light);
		++count;
		return light;
	}
	return nullptr;
}



void Light::Destroy(Light* light)
{
	Light* lastLight = (lights.back());

	if (lights.size() > 1)
	{
		//std::swap(light, lastLight);

		light->aperture = lastLight->aperture;
		light->attenuation = lastLight->attenuation;
		light->radius = lastLight->radius;
		light->focus = lastLight->focus;
		light->color = lastLight->color;
		light->direction = lastLight->direction;
		light->position = lastLight->position;
		light->intensity = lastLight->intensity;
		light->name = lastLight->name;
		light->type = lastLight->type;

		//lightToDel = lastLight;
	}

	lights.remove(lastLight);
	delete lastLight;
	--count;
}

void Light::Init()
{
	count = 0;
}

void Light::SendData(Shader* shader)
{
	shader->setVectorArray("lightColors", count, 4, colors);
	shader->setVectorArray("lightAttribs", count, 4, attribs);
	shader->setVectorArray("lightDirections", count, 3, directions);
	shader->setVectorArray("lightPositions", count, 3, positions);
	shader->setIntArray("lightTypes", count*3, types);
	shader->setInt("lightCount", count);
}

std::string ToString(LightType type)
{
	switch (type)
	{
	case 0:
		return "None";
	case 1:
		return "Directional";
	case 2:
		return "Point";
	case 3:
		return "Spot";
	default:
		return "None";
	}
}
