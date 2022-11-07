#pragma once

#include <string>
#include <list>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include "Input/Delegate.h"

class Texture;
class Shader;

class Material
{
public:

	std::string name;
	Texture* finalTexture;
	Texture* diffuse;
	Texture* roughness;
	Texture* normal;
	Texture* ambientOcclusion;
	int id;
	bool needMerge;

	~Material();

	void Merge();


	static std::list<Material*> materials;
	static bool needMergeBeforeSending;

	static Material* Create();
	static void SendData(Shader* shader);
	static void Save(std::string& outSave);
	static void Load(std::stringstream& inSave);
	static void Clear();

	static Notify onClear;

private:

	Material();

	glm::mat2x4 textureSizes;


};

