#include "ShaderLeaf.h"
#include <iostream>
#include <sstream>
#include <string>
#include "Shader.h"
#include "ShaderGenerator.h"

ShapeData ShaderLeaf::dynData[DYNAMIC_DATA_NUM];
glm::vec3 ShaderLeaf::dynColor[DYNAMIC_DATA_NUM];
glm::vec3 ShaderLeaf::dynLightingProps[DYNAMIC_DATA_NUM];
glm::vec3 ShaderLeaf::dynPalette[DYNAMIC_DATA_NUM * 4];
std::list<ShaderLeaf*> ShaderLeaf::dynLeaves;
int ShaderLeaf::dynLeavesCount;

ShaderLeaf::ShaderLeaf()
{
	dynDataID = -1;
	shapeData = new ShapeData();
	color = new Vector3();
	lightingProperties = new Vector3(0, 1, 1);
	palette = new Vector3[4];
	customPimitive = nullptr;
	useColorPalette = false;
	SetShape(PrimitiveShape::Box);
}

ShaderLeaf::ShaderLeaf(ShaderPart* _parent) : ShaderPart(_parent)
{
	dynDataID = -1;
	shapeData = new ShapeData();
	color = new Vector3();
	lightingProperties = new Vector3(0, 1, 1);
	palette = new Vector3[4];
	customPimitive = nullptr;
	useColorPalette = false;
	SetShape(PrimitiveShape::Box);
}

ShaderLeaf::~ShaderLeaf()
{
	if (IsDynamic()) SetDynamic(false);
	delete shapeData;
	delete color;
	delete lightingProperties;
	delete[] palette;
	delete customPimitive;
}

void ShaderLeaf::ResetShape(ShaderFunction* func)
{
	customPimitive = nullptr;
	SetShape(PrimitiveShape::Box);
}

void ShaderLeaf::SetShape(PrimitiveShape newShape)
{
	if (newShape == shape) return;

	if (shape == PrimitiveShape::Custom)
	{
		if (customPimitive) delete customPimitive;
		customPimitive = nullptr;
	}

	shape = newShape;

	switch (shape)
	{
	case PrimitiveShape::Box:
		shapeData->box.halfExtend = Vector3(0.5f, 0.5f, 0.5f);
		break;
	case PrimitiveShape::BoxFrame:
		shapeData->boxFrame.edgeThickness = 0.05f;
		shapeData->boxFrame.halfExtend = Vector3(0.5f, 0.5f, 0.5f);
		break;
	case PrimitiveShape::CappedTorus:
		shapeData->cappedTorus.mainRadius = 0.5f;
		shapeData->cappedTorus.auxRadius = 0.1f;
		shapeData->cappedTorus.angle = 0;
		break;
	case PrimitiveShape::Capsule:
		shapeData->capsule.height = 1.0f;
		shapeData->capsule.radius = 0.2f;
		break;
	case PrimitiveShape::Cone:
		shapeData->cone.angle = 3.1415f / 4.0f;
		shapeData->cone.height = 1.0f;
		break;
	case PrimitiveShape::Cylinder:
		shapeData->cylinder.height = 1.0f;
		shapeData->cylinder.radius = 0.5f;
		break;
	case PrimitiveShape::Link:
		shapeData->link.mainRadius = 0.5f;
		shapeData->link.auxRadius = 0.1f;
		shapeData->link.length = 0.5f;
		break;
	case PrimitiveShape::Plane:
		break;
	case PrimitiveShape::Sphere:
		shapeData->sphere.radius = 0.5f;
		break;
	case PrimitiveShape::Torus:
		shapeData->torus.mainRadius = 0.5f;
		shapeData->torus.auxRadius = 0.5f;
		break;
	case PrimitiveShape::Custom:
	{
		if (ShaderFunction::models[FunctionType::Primitive].size() > 0)
		{
			ShaderFunction* func = new ShaderFunction(**ShaderFunction::models[FunctionType::Primitive].begin());
			SetCustomShape(func);
			break;
		}
	}
	default:
		shape = PrimitiveShape::Box;
		shapeData->box.halfExtend = Vector3(0.5f, 0.5f, 0.5f);
		break;
	}
}

void ShaderLeaf::SetCustomShape(ShaderFunction* _customShape)
{
	if (customPimitive && (_customShape && &_customShape->model == &customPimitive->model))
	{
		delete _customShape;
		return;
	}

	bool dyn = IsDynamic();

	if (customPimitive)
	{
		customPimitive->OnModelWillDestroy.Unbind(this, &ShaderLeaf::ResetShape);
		delete customPimitive;
	}

	customPimitive = _customShape;
	if (customPimitive)
	{
		customPimitive->OnModelWillDestroy.Bind(this, &ShaderLeaf::ResetShape);
		customPimitive->SetDynamic(dyn);
	}
}

void SaveData(std::string& outSave, SphereData sphere)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Sphere) + "\n";
	outSave += "radius " + STR(sphere.radius) + "\n";
}

void LoadData(std::stringstream& inSave, SphereData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "radius")
		{
			data->radius = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, BoxData box)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Box) + "\n";
	outSave += "halfExtends " + STR(box.halfExtend.x) + " " + STR(box.halfExtend.y) + " " + STR(box.halfExtend.z) + "\n";
}

void LoadData(std::stringstream& inSave, BoxData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "halfExtends")
		{
			data->halfExtend = Vector3::Parse(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, BoxFrameData boxFrame)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::BoxFrame) + "\n";
	outSave += "halfExtends " + STR(boxFrame.halfExtend.x) + " " + STR(boxFrame.halfExtend.y) + " " + STR(boxFrame.halfExtend.z) + "\n";
	outSave += "edgeThickness " + STR(boxFrame.edgeThickness) + "\n";
}

void LoadData(std::stringstream& inSave, BoxFrameData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "halfExtends")
		{
			data->halfExtend = Vector3::Parse(value);
		}
		else if (word == "edgeThickness")
		{
			data->edgeThickness = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, TorusData torus)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Torus) + "\n";
	outSave += "mainRadius " + STR(torus.mainRadius) + "\n";
	outSave += "auxRadius " + STR(torus.auxRadius) + "\n";
}

void LoadData(std::stringstream& inSave, TorusData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "mainRadius")
		{
			data->mainRadius = std::stof(value);
		}
		else if (word == "auxRadius")
		{
			data->auxRadius = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, CappedTorusData cappedTorus)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::CappedTorus) + "\n";
	outSave += "mainRadius " + STR(cappedTorus.mainRadius) + "\n";
	outSave += "auxRadius " + STR(cappedTorus.auxRadius) + "\n";
	outSave += "angle " + STR(cappedTorus.angle) + "\n";
}

void LoadData(std::stringstream& inSave, CappedTorusData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "mainRadius")
		{
			data->mainRadius = std::stof(value);
		}
		else if (word == "auxRadius")
		{
			data->auxRadius = std::stof(value);
		}
		else if (word == "angle")
		{
			data->angle = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, LinkData link)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Link) + "\n";
	outSave += "mainRadius " + STR(link.mainRadius) + "\n";
	outSave += "auxRadius " + STR(link.auxRadius) + "\n";
	outSave += "length " + STR(link.length) + "\n";
}

void LoadData(std::stringstream& inSave, LinkData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "mainRadius")
		{
			data->mainRadius = std::stof(value);
		}
		else if (word == "auxRadius")
		{
			data->auxRadius = std::stof(value);
		}
		else if (word == "length")
		{
			data->length = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, ConeData cone)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Cone) + "\n";
	outSave += "angle " + STR(cone.angle) + "\n";
	outSave += "height " + STR(cone.height) + "\n";
}

void LoadData(std::stringstream& inSave, ConeData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "height")
		{
			data->height = std::stof(value);
		}
		else if (word == "angle")
		{
			data->angle = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, CapsuleData capsule)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Capsule) + "\n";
	outSave += "height " + STR(capsule.height) + "\n";
	outSave += "radius " + STR(capsule.radius) + "\n";
}

void LoadData(std::stringstream& inSave, CapsuleData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "height")
		{
			data->height = std::stof(value);
		}
		else if (word == "radius")
		{
			data->radius = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void SaveData(std::string& outSave, CylinderData cylinder)
{
	outSave += "shape " + STR((unsigned int)PrimitiveShape::Cylinder) + "\n";
	outSave += "height " + STR(cylinder.height) + "\n";
	outSave += "radius " + STR(cylinder.radius) + "\n";
}

void LoadData(std::stringstream& inSave, CylinderData* data)
{
	bool stop = false;
	char buf[512];
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "height")
		{
			data->height = std::stof(value);
		}
		else if (word == "radius")
		{
			data->radius = std::stof(value);
		}
		else if (word == "end")
		{
			stop = true;
		}
	}
}

void LoadCustomData(std::stringstream& inSave, ShaderLeaf* leaf)
{
	char buf[512];
	
	inSave.getline(buf, 512, '\n');
	std::string line = buf;
	size_t fo = line.find_first_of(' ');
	fo = (fo == std::string::npos) ? line.size() : fo;
	std::string word = std::string(line.c_str(), fo);
	std::string value = std::string(line.c_str() + fo + 1);

	ShaderFunctionModel* model = ShaderFunction::GetModel(value);
	ShaderFunction* func = new ShaderFunction(*model);
	func->Load(inSave);
	leaf->SetShape(PrimitiveShape::Custom);
	leaf->SetCustomShape(func);
}

void ShaderLeaf::Save(std::string& outSave)
{
	outSave += "new object\n";
	ShaderPart::Save(outSave);
	outSave += "dynData " + STR(IsDynamic()) + "\n";

	outSave += "lightProps " + STR(lightingProperties->x) + " " + STR(lightingProperties->y) + " " + STR(lightingProperties->z) + "\n";
	
	if (useColorPalette)
		outSave += "colorP " + STR(palette[0].x) + " " + STR(palette[0].y) + " " + STR(palette[0].z) + ";" + STR(palette[1].x) + " " + STR(palette[1].y) + " " + STR(palette[1].z) + ";" + STR(palette[2].x) + " " + STR(palette[2].y) + " " + STR(palette[2].z) + ";" + STR(palette[3].x) + " " + STR(palette[3].y) + " " + STR(palette[3].z) + "\n";
	else
		outSave += "color " + STR(color->x) + " " + STR(color->y) + " " + STR(color->z) + "\n";

	// TODO : Add custom primitive handling

	switch (shape)
	{
	case PrimitiveShape::Box:
		SaveData(outSave, shapeData->box);
		break;
	case PrimitiveShape::BoxFrame:
		SaveData(outSave, shapeData->boxFrame);
		break;
	case PrimitiveShape::CappedTorus:
		SaveData(outSave, shapeData->cappedTorus);
		break;
	case PrimitiveShape::Capsule:
		SaveData(outSave, shapeData->capsule);
		break;
	case PrimitiveShape::Cone:
		SaveData(outSave, shapeData->cone);
		break;
	case PrimitiveShape::Cylinder:
		SaveData(outSave, shapeData->cylinder);
		break;
	case PrimitiveShape::Link:
		SaveData(outSave, shapeData->link);
		break;
	case PrimitiveShape::Plane:
		outSave += "shape " + STR((unsigned int)PrimitiveShape::Plane) + "\n";
		break;
	case PrimitiveShape::Sphere:
		SaveData(outSave, shapeData->sphere);
		break;
	case PrimitiveShape::Torus:
		SaveData(outSave, shapeData->torus);
		break;
	case PrimitiveShape::Custom:
		outSave += "shape " + STR((unsigned int)PrimitiveShape::Custom) + "\n";
		customPimitive->Save(outSave);
		break;
	}
	outSave += "end\n";
}

void ParsePalette(std::string& value, Vector3 palette[4])
{
	std::string tmp;
	tmp = value;
	for (int i = 0; i < 4; ++i)
	{
		size_t fo = tmp.find_first_of(';');
		fo = (fo == std::string::npos) ? tmp.size() : fo;
		std::string word = std::string(tmp.c_str(), fo);
		std::string value = std::string(tmp.c_str() + fo + 1);
		palette[i] = Vector3::Parse(word);
		tmp = value;
	}
}

void ShaderLeaf::Load(std::stringstream& inSave)
{
	ShaderPart::Load(inSave);

	char buf[512];
	for (int i = 0; i < 3; i++)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "dynData")
		{
			SetDynamic((bool)std::stoi(value));
		}
		else if (word == "lightProps")
		{
			i--;
			*lightingProperties = Vector3::Parse(value);
		}
		else if (word == "color")
		{
			useColorPalette = false;
			*color = Vector3::Parse(value);
		}
		else if (word == "colorP")
		{
			useColorPalette = true;
			ParsePalette(value, palette);
		}
		else if (word == "shape")
		{
			shape = (PrimitiveShape)std::stoi(value);

			switch (shape)
			{
			case PrimitiveShape::Box:
				LoadData(inSave, &shapeData->box);
				return;
			case PrimitiveShape::BoxFrame:
				LoadData(inSave, &shapeData->boxFrame);
				return;
			case PrimitiveShape::CappedTorus:
				LoadData(inSave, &shapeData->cappedTorus);
				return;
			case PrimitiveShape::Capsule:
				LoadData(inSave, &shapeData->capsule);
				return;
			case PrimitiveShape::Cone:
				LoadData(inSave, &shapeData->cone);
				return;
			case PrimitiveShape::Cylinder:
				LoadData(inSave, &shapeData->cylinder);
				return;
			case PrimitiveShape::Link:
				LoadData(inSave, &shapeData->link);
				return;
			case PrimitiveShape::Sphere:
				LoadData(inSave, &shapeData->sphere);
				return;
			case PrimitiveShape::Torus:
				LoadData(inSave, &shapeData->torus);
				return;
			case PrimitiveShape::Custom:
				LoadCustomData(inSave, this);
				return;
			default:
				return;
			}
		}
	}
}

ShaderPart* ShaderLeaf::Copy()
{
	ShaderLeaf* leaf = (ShaderLeaf*)ShaderPart::Copy();

	leaf->useColorPalette = useColorPalette;
	leaf->color = new Vector3(*color);
	leaf->lightingProperties = new Vector3(*lightingProperties);
	leaf->palette = new Vector3[4];
	memcpy(leaf->palette, palette, sizeof(Vector3) * 4);
	leaf->shapeData = new ShapeData(*shapeData);
	leaf->dynDataID = -1;

	if (customPimitive)
	{
		leaf->customPimitive = nullptr;
		leaf->SetCustomShape(new ShaderFunction(*customPimitive));
		//leaf->customPimitive = new ShaderFunction(*customPimitive);
	}

	leaf->SetDynamic(IsDynamic());

	return leaf;
}

bool ShaderLeaf::IsDynamic()
{
	return (dynDataID >= 0) && (customPimitive == nullptr || customPimitive->IsDynamic());
}

Vector3* CopyPalette(Vector3* _palette)
{
	Vector3* newPalette = new Vector3[4];
	for (int i = 0; i < 4; ++i)
	{
		newPalette[i] = _palette[i];
	}
	return newPalette;
}

bool ShaderLeaf::RequestDynamic(ShaderLeaf* leaf)
{
	if (/*!leaf->IsDynamic() &&*/ dynLeavesCount < DYNAMIC_DATA_NUM)
	{
		ShapeData* old = leaf->shapeData;
		Vector3* oldColor = leaf->color;
		Vector3* oldLightingProperties = leaf->lightingProperties;
		Vector3* oldPalette = leaf->palette;
		leaf->shapeData = &dynData[dynLeavesCount];
		leaf->color = (Vector3*)&dynColor[dynLeavesCount];
		leaf->lightingProperties = (Vector3*)&dynLightingProps[dynLeavesCount];
		leaf->palette = (Vector3*)&dynPalette[dynLeavesCount * 4];
		leaf->shapeData->boxFrame = old->boxFrame;
		*leaf->color = *oldColor;
		*leaf->lightingProperties = *oldLightingProperties;
		memcpy(leaf->palette, oldPalette, sizeof(Vector3) * 4);
		delete old;
		delete oldColor;
		delete oldLightingProperties;
		delete[] oldPalette;
		leaf->dynDataID = dynLeavesCount;
		dynLeaves.push_back(leaf);
		dynLeavesCount++;
		return true;
	}
	else
	{
		return false;
	}
}

void ShaderLeaf::ReleaseDynamic(ShaderLeaf* leaf)
{
	//if (leaf->IsDynamic())
	{
		BoxFrameData old = leaf->shapeData->boxFrame;
		Vector3 oldColor = *leaf->color;
		Vector3 oldLightingProperties = *leaf->lightingProperties;
		Vector3* oldPalette = CopyPalette(leaf->palette);

		if (dynLeaves.size() > 1)
		{
			ShaderLeaf* lastLeaf = nullptr;

			for (auto tmpLeaf : dynLeaves)
			{
				if (!lastLeaf || tmpLeaf->dynDataID > lastLeaf->dynDataID)
				{
					lastLeaf = tmpLeaf;
				}
			}

			if (lastLeaf)
			{
				BoxFrameData tmp = lastLeaf->shapeData->boxFrame;
				Vector3 tmpColor = *lastLeaf->color;
				Vector3 tmpLightingProperties = *lastLeaf->lightingProperties;
				Vector3* tmpPalette = CopyPalette(lastLeaf->palette);
				lastLeaf->shapeData = leaf->shapeData;
				lastLeaf->color = leaf->color;
				lastLeaf->lightingProperties = leaf->lightingProperties;
				lastLeaf->palette = leaf->palette;
				lastLeaf->shapeData->boxFrame = tmp;
				*lastLeaf->color = tmpColor;
				*lastLeaf->lightingProperties = tmpLightingProperties;
				memcpy(lastLeaf->palette, tmpPalette, sizeof(Vector3) * 4);
				delete[] tmpPalette;
				lastLeaf->dynDataID = leaf->dynDataID;
			}
		}
		leaf->shapeData = new ShapeData();
		leaf->color = new Vector3();
		leaf->lightingProperties = new Vector3();
		leaf->shapeData->boxFrame = old;
		leaf->palette = oldPalette;
		*leaf->lightingProperties = oldLightingProperties;
		leaf->dynDataID = -1;
		dynLeaves.remove(leaf);
		dynLeavesCount--;
	}
}

void ShaderLeaf::SetDynamic(bool dynamic)
{
	if (IsDynamic() == dynamic) return;

	if (customPimitive && !customPimitive->SetDynamic(dynamic)) return;

	if (dynamic)
	{
		RequestDynamic(this);
	}
	else
	{
		ReleaseDynamic(this);
	}
}

void ShaderLeaf::Init()
{
	dynLeavesCount = 0;
}

void ShaderLeaf::SendData(Shader* shader)
{
	shader->setVectorArray("shapeData", DYNAMIC_DATA_NUM * 4, 4, (float*)dynData);
	shader->setVectorArray("shapeColors", DYNAMIC_DATA_NUM * 3, 3, (float*)dynColor);
	shader->setVectorArray("lightingProperties", DYNAMIC_DATA_NUM * 3, 3, (float*)dynLightingProps);
	shader->setVectorArray("shapePalettes", DYNAMIC_DATA_NUM * 3 * 4, 3, (float*)dynPalette);
}

std::string ShaderLeaf::GetColorCode()
{
	//a + b * cos[2PI(c*t + d)];
	if (IsDynamic())
	{
		if (useColorPalette)
		{
			return "shapePalettes[" + STR(dynDataID * 4) + "] + \n"
				"shapePalettes[" + STR(dynDataID * 4 + 1) + "] * \n"
				"cos(2.0f * 3.1415f * (shapePalettes[" + STR(dynDataID * 4 + 2) + "] * gradientValue + \n"
				"shapePalettes[" + STR(dynDataID * 4 + 3) + "]))";
		}
		else
		{
			return "shapeColors[" + STR(dynDataID) + "]";
		}
	}
	else
	{
		if (useColorPalette)
		{
			return "vec3(" + STR(palette[0].x) + "," + STR(palette[0].y) + "," + STR(palette[0].z) + ") + \n"
				"vec3(" + STR(palette[1].x) + "," + STR(palette[1].y) + "," + STR(palette[1].z) + ") * \n"
				"cos(2.0f * 3.1415f * (vec3(" + STR(palette[2].x) + "," + STR(palette[2].y) + "," + STR(palette[2].z) + ") * gradientValue + \n"
				"vec3(" + STR(palette[3].x) + "," + STR(palette[3].y) + "," + STR(palette[3].z) + ")))";
		}
		else
		{
			return "vec3(" + STR(color->x) + "," + STR(color->y) + "," + STR(color->z) + ")";
		}
	}
}

std::string ShaderLeaf::GetLightingPropertiesCode()
{
	//a + b * cos[2PI(c*t + d)];
	if (IsDynamic())
	{
		return "lightingProperties[" + STR(dynDataID) + "]";
	}
	else
	{
		return "vec3(" + STR(lightingProperties->x) + "," + STR(lightingProperties->y) + "," + STR(lightingProperties->z) + ")";
	}
}

void ShaderLeaf::DrawShape(std::string& outCode, GenerationPass pass)
{
	if (IsDynamic())
	{
		outCode += Tab(layer + 2) + "tmpDist = ";

		switch (shape)
		{
		case PrimitiveShape::Box:
			outCode += "sdBox( tmpP" + Layer + ", shapeData[" + STR(dynDataID) + "].xyz);\n";
			break;
		case PrimitiveShape::BoxFrame:
			outCode += "sdBoxFrame( tmpP" + Layer + ", shapeData[" + STR(dynDataID) + "].xyz, shapeData[" + STR(dynDataID) + "].w ); \n ";
			break;
		case PrimitiveShape::CappedTorus:
			outCode += "sdCappedTorus( tmpP" + Layer + ", vec2(sin(shapeData[" + STR(dynDataID) + "].z), cos(shapeData[" + STR(dynDataID) + "].z)), shapeData[" + STR(dynDataID) + "].x, shapeData[" + STR(dynDataID) + "].y ); \n";
			break;
		case PrimitiveShape::Capsule:
			outCode += "sdCapsule( tmpP" + Layer + ", shapeData[" + STR(dynDataID) + "].x, shapeData[" + STR(dynDataID) + "].y );\n";
			break;
		case PrimitiveShape::Cone:
			outCode += "sdCone( tmpP" + Layer + ", vec2(cos(shapeData[" + STR(dynDataID) + "].x), sin(shapeData[" + STR(dynDataID) + "].x)), shapeData[" + STR(dynDataID) + "].y );\n";
			break;
		case PrimitiveShape::Cylinder:
			outCode += "sdCappedCylinder( tmpP" + Layer + ", shapeData[" + STR(dynDataID) + "].y, shapeData[" + STR(dynDataID) + "].x );\n";
			break;
		case PrimitiveShape::Link:
			outCode += "sdLink( tmpP" + Layer + ", shapeData[" + STR(dynDataID) + "].x, shapeData[" + STR(dynDataID) + "].y, shapeData[" + STR(dynDataID) + "].z );\n";
			break;
		case PrimitiveShape::Plane:
			outCode += "sdPlane( tmpP" + Layer + " );\n";
			break;
		case PrimitiveShape::Sphere:
			outCode += "SignedDistToSphere( tmpP" + Layer + ", shapeData[" + STR(dynDataID) + "].x );\n";
			break;
		case PrimitiveShape::Torus:
			outCode += "sdTorus( tmpP" + Layer + ", vec2(shapeData[" + STR(dynDataID) + "].x, shapeData[" + STR(dynDataID) + "].y) );\n";
			break;
		case PrimitiveShape::Custom:
		{
			if (customPimitive)
			{
				outCode += customPimitive->GenerateFunctionCall(layer) + ";\n";
				break;
			}
		}
		default:
			outCode += "99999999;\n";
			break;
		}

		if (pass == Color)
		{
			outCode += (Tab(layer + 2) + "color" + Layer + " = " + GetColorCode() + ";\n");
		}
		else if (pass == LightingProp)
		{
			outCode += (Tab(layer + 2) + "lp" + Layer + " = " + GetLightingPropertiesCode() + ";\n");
		}
		else
		{
			if (emittingLight) outCode += Tab(layer + 2) + "AddGlow(" + GetColorCode() + ", tmpDist);\n";
		}
	}
	else
	{
		outCode += Tab(layer + 2) + "tmpDist = ";

		switch (shape)
		{
		case PrimitiveShape::Box:
			outCode += "sdBox( tmpP" + Layer + ", vec3(" + STR(shapeData->box.halfExtend.x) + "," + STR(shapeData->box.halfExtend.y) + "," + STR(shapeData->box.halfExtend.z) + "));\n";
			break;
		case PrimitiveShape::BoxFrame:
			outCode += "sdBoxFrame( tmpP" + Layer + ", vec3(" + STR(shapeData->boxFrame.halfExtend.x) + "," + STR(shapeData->boxFrame.halfExtend.y) + "," + STR(shapeData->boxFrame.halfExtend.z) + "), " + STR(shapeData->boxFrame.edgeThickness) + " ); \n ";
			break;
		case PrimitiveShape::CappedTorus:
			outCode += "sdCappedTorus( tmpP" + Layer + ", vec2(" + STR(sin(shapeData->cappedTorus.angle)) + "," + STR(cos(shapeData->cappedTorus.angle)) + "), " + STR(shapeData->cappedTorus.mainRadius) + ", " + STR(shapeData->cappedTorus.auxRadius) + " ); \n";
			break;
		case PrimitiveShape::Capsule:
			outCode += "sdCapsule( tmpP" + Layer + ", " + STR(shapeData->capsule.height) + ", " + STR(shapeData->capsule.radius) + " );\n";
			break;
		case PrimitiveShape::Cone:
			outCode += "sdCone( tmpP" + Layer + ", vec2(" + STR(cos(shapeData->cone.angle)) + "," + STR(sin(shapeData->cone.angle)) + "), " + STR(shapeData->cone.height) + " );\n";
			break;
		case PrimitiveShape::Cylinder:
			outCode += "sdCappedCylinder( tmpP" + Layer + ", " + STR(shapeData->cylinder.radius) + ", " + STR(shapeData->cylinder.height) + " );\n";
			break;
		case PrimitiveShape::Link:
			outCode += "sdLink( tmpP" + Layer + ", " + STR(shapeData->link.length) + ", " + STR(shapeData->link.mainRadius) + ", " + STR(shapeData->link.auxRadius) + " );\n";
			break;
		case PrimitiveShape::Plane:
			outCode += "sdPlane( tmpP" + Layer + " );\n";
			break;
		case PrimitiveShape::Sphere:
			outCode += "SignedDistToSphere( tmpP" + Layer + ", " + STR(shapeData->sphere.radius) + " );\n";
			break;
		case PrimitiveShape::Torus:
			outCode += "sdTorus( tmpP" + Layer + ", vec2(" + STR(shapeData->torus.mainRadius) + "," + STR(shapeData->torus.auxRadius) + ") );\n";
			break;
		case PrimitiveShape::Custom:
		{
			if (customPimitive)
			{
				outCode += customPimitive->GenerateFunctionCall(layer) + ";\n";
				break;
			}
		}
		default:
			outCode += "99999999;\n";
			break;
		}

		if (pass == Color)
		{
			outCode += (Tab(layer + 2) + "color" + Layer + " = " + GetColorCode() + ";\n");
		}
		else if (pass == LightingProp)
		{
			outCode += (Tab(layer + 2) + "lp" + Layer + " = " + GetLightingPropertiesCode() + ";\n");
		}
		else
		{
			if (emittingLight) outCode += Tab(layer + 2) + "AddGlow(" + GetColorCode() + ", tmpDist);\n\n";
		}
	}
}

ShapeData::ShapeData()
{
	box.halfExtend = Vector3(0.5f, 0.5f, 0.5f);
}

ShapeData::~ShapeData()
{

}

std::string ToString(PrimitiveShape& shape)
{
	switch (shape)
	{
	case PrimitiveShape::Box:
		return std::string("Box");
		break;
	case PrimitiveShape::BoxFrame:
		return std::string("Box Frame");
		break;
	case PrimitiveShape::CappedTorus:
		return std::string("Capped Torus");
		break;
	case PrimitiveShape::Capsule:
		return std::string("Capsule");
		break;
	case PrimitiveShape::Cone:
		return std::string("Cone");
		break;
	case PrimitiveShape::Cylinder:
		return std::string("Cylinder");
		break;
	case PrimitiveShape::Link:
		return std::string("Link");
		break;
	case PrimitiveShape::Plane:
		return std::string("Plane");
		break;
	case PrimitiveShape::Sphere:
		return std::string("Sphere");
		break;
	case PrimitiveShape::Torus:
		return std::string("Torus");
		break;
	case PrimitiveShape::Custom:
		return std::string("Custom");
		break;
	default:
		return std::string("ERROR");
		break;
	}
}
