#pragma once
#include "Vector3.h"
#include "Transformable.h"
#include <list>
#include "Transform.h"

class Lighting;
class Shader;

enum LightType
{
	none,
	directional,
	point,
	spot,
	count
};

std::string ToString(LightType type);

class Light : public Transformable
{

public:

	// object

	std::string name;
	Vector3& position;
	Vector3& direction;
	LightType& type;
	Vector3& color;
	float& intensity;
	float& radius;
	float& attenuation;
	float& aperture;
	float& focus;
	int ID;
	Transform transform;

	void Save(std::string& outSave);
	void Load(std::stringstream& inSave);

	// class

	static std::list<Light*> lights;

	static Light* Create();
	static void Destroy(Light* light);
	static void Clear();
	static void Init();
	static void SendData(Shader* shader);

	virtual glm::mat4 GetLocalBase() override;
	virtual glm::vec3 GetPosition() override;
	virtual glm::vec3 GetRotation() override;
	virtual glm::vec3 GetScale() override;
	virtual void SetPosition(glm::vec3 _position) override;
	virtual void SetRotation(glm::vec3 _rotation) override;
	virtual void SetScale(glm::vec3 _scale) override;
	virtual bool IsDynamic() override;

private:

	Light();
	~Light();

	static int count;
	static int types[32];
	static float positions[96];
	static float directions[96];
	static float colors[128];
	static float attribs[128];

};

