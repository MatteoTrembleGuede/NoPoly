#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include <iostream>
#include "UIResourceBrowser.h"
#include "ShaderGenerator.h"

std::list<Material*> Material::materials;
bool Material::needMergeBeforeSending;
Notify Material::onClear;

Material::Material()
{
	name = "new material";

	diffuse = nullptr;
	normal = nullptr;
	ambientOcclusion = nullptr;
	roughness = nullptr;
	finalTexture = nullptr;
	id = materials.size();

	materials.push_back(this);
	needMergeBeforeSending = true;
	needMerge = true;
}

Material::~Material()
{
	delete diffuse;
	delete normal;
	delete ambientOcclusion;
	delete roughness;
	delete finalTexture;

	materials.remove(this);
}

void Material::Merge()
{
	if (finalTexture)
	{
		delete finalTexture;
		finalTexture = nullptr;
	}
	unsigned int maxWidth = 1;
	unsigned int maxHeight = 1;
	ColorChannel format = RGB;
	unsigned char* buffer;
	int id = 0;

	for (Texture** tex = &diffuse; tex != &ambientOcclusion; tex = tex + 1)
	{
		if (!(*tex)) continue;
		unsigned int tmpWidth, tmpHeight;
		(*tex)->GetSize(tmpWidth, tmpHeight);

		textureSizes[0][id] = tmpWidth;
		textureSizes[1][id] = tmpHeight;

		if (maxWidth < tmpWidth) maxWidth = tmpWidth;
		if (maxHeight < tmpHeight) maxHeight = tmpHeight;

		++id;
	}

	for (int i = 0; i < 4; ++i)
	{
		textureSizes[0][i] /= float(maxWidth);
		textureSizes[1][i] /= float(maxHeight);
	}

	buffer = new unsigned char[maxWidth * maxHeight * 12];
	memset(buffer, 255, maxWidth * maxHeight * 12);

	finalTexture = Texture::LoadFromBuffer(format, maxWidth * 2, maxHeight * 2, buffer);

	for (unsigned int t = 0; t < 4; ++t)
	{
		Texture* tex = *(&diffuse + t);
		if (!tex) continue;

		unsigned int offX = (t / 2);
		unsigned int offY = (t % 2);
		unsigned int width, height;
		tex->GetSize(width, height);

		for (unsigned int l = 0; l < height; ++l)
		{
			unsigned int coordSrc = l * 3 * width;
			unsigned int coordDst = ((l + offY * maxHeight) * maxWidth * 2 + (offX * maxWidth)) * 3;
			memcpy(finalTexture->GetRawBuffer() + coordDst, tex->GetRawBuffer() + coordSrc, width * 3);
		}
	}

	finalTexture->UpdateBuffer();
	needMerge = false;
}

Material* Material::Create()
{
	Material* newMat = nullptr;
	if (materials.size() < 32)
	{
		newMat = new Material();
	}
	else
	{
		std::cout << "can't have more than 32 materials" << std::endl;
	}

	return newMat;
}

void Material::SendData(Shader* shader)
{
	int textureIDs[32];
	glm::mat2x4 textureSizes[32];
	shader->use();

	if (needMergeBeforeSending)
	{
		for (Material* mat : materials)
		{
			if (mat->needMerge)
			{
				mat->Merge();
			}
		}
		needMergeBeforeSending = false;
	}

	/*for (Material* mat : materials)
	{
		textureIDs[mat->id] = mat->id;
		textureSizes[mat->id] = mat->textureSizes;
		mat->finalTexture->Bind(mat->id);
	}*/

	for (Material* mat : materials)
	{
		shader->setInt((std::string("materials[") + std::to_string(mat->id) + "]"), mat->id);
		textureIDs[mat->id] = mat->id;
		textureSizes[mat->id] = mat->textureSizes;
		mat->finalTexture->Bind(mat->id);
	}

	//shader->setIntArray("materials", 32, textureIDs);
	shader->setMat2x4Array("materialTextureSizes", 32, textureSizes);
}

void Material::Save(std::string& outSave)
{
	outSave += std::string("materialNumber ") + std::to_string(materials.size()) + "\n";
	for (Material* mat : materials)
	{
		outSave += "name " + mat->name + "\n";
		outSave += "id " + std::to_string(mat->id) + "\n";
		outSave += "diffuse " + (mat->diffuse ? UIResourceBrowser::GetRelativePath(mat->diffuse->GetPath(), ShaderGenerator::GetProjectPath()) : "none") + "\n";
		outSave += "normal " + (mat->normal ? UIResourceBrowser::GetRelativePath(mat->normal->GetPath(), ShaderGenerator::GetProjectPath()) : "none") + "\n";
		outSave += "roughness " + (mat->roughness ? UIResourceBrowser::GetRelativePath(mat->roughness->GetPath(), ShaderGenerator::GetProjectPath()) : "none") + "\n";
		outSave += "ambientOcclusion " + (mat->ambientOcclusion ? UIResourceBrowser::GetRelativePath(mat->ambientOcclusion->GetPath(), ShaderGenerator::GetProjectPath()) : "none") + "\n";
	}
}

void Material::Load(std::stringstream& inSave)
{
	Material::Clear();
	char buf[512];
	std::string line;
	size_t fo;
	std::string word;
	std::string value;

	inSave.getline(buf, 512, '\n');
	line = buf;
	fo = line.find_first_of(' ');
	fo = (fo == std::string::npos) ? line.size() : fo;
	word = std::string(line.c_str(), fo);
	value = std::string(line.c_str() + fo + 1);


	if (word == "materialNumber")
	{
		needMergeBeforeSending = true;
		int matNum = std::stoi(value);
		for (int i = 0; i < matNum; ++i)
		{
			Material* newMat = new Material();
			newMat->needMerge = true;
			for (int j = 0; j < 6; ++j)
			{
				inSave.getline(buf, 512, '\n');
				line = buf;
				fo = line.find_first_of(' ');
				fo = (fo == std::string::npos) ? line.size() : fo;
				word = std::string(line.c_str(), fo);
				value = std::string(line.c_str() + fo + 1);

				if (word == "name")
				{
					newMat->name = value;
				}
				else if (word == "id")
				{
					newMat->id = std::stoi(value);
				}
				else if (word == "diffuse")
				{
					newMat->diffuse = Texture::LoadFromFile((ShaderGenerator::GetProjectPath() + value).c_str());
				}
				else if (word == "normal")
				{
					newMat->normal = Texture::LoadFromFile((ShaderGenerator::GetProjectPath() + value).c_str());
				}
				else if (word == "roughness")
				{
					newMat->roughness = Texture::LoadFromFile((ShaderGenerator::GetProjectPath() + value).c_str());
				}
				else if (word == "ambientOcclusion")
				{
					newMat->ambientOcclusion = Texture::LoadFromFile((ShaderGenerator::GetProjectPath() + value).c_str());
				}
			}
		}
	}
}

void Material::Clear()
{
	auto mat = materials.begin();
	while (mat != materials.end())
	{
		Material* tmp = *mat;
		++mat;
		delete tmp;
	}
	materials.clear();
	onClear();
}
