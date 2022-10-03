#include "CustomPrimitive.h"
#include "Shader.h"
#include "UIResourceBrowser.h"
#include <map>
#include <utility>
#include <iostream>
#include "ShaderGenerator.h"

AttribType ToAttribType(std::string attribType);
FunctionType ToFunctionType(std::string funcType);

// PRIMITIVE MODEL //

std::list<ShaderFunctionModel*> ShaderFunction::models[FunctionType::FT_Count];
float ShaderFunction::customData[CUSTOM_DATA_NUM];
bool  ShaderFunction::availabilities[CUSTOM_DATA_NUM];

void ShaderFunction::SendData(Shader* shader)
{
	shader->setFloatArray("customData", CUSTOM_DATA_NUM, customData);

	/*system("cls");
	for (int x = 0; x < 16; ++x)
	{
		for (int i = 0; i < 32; ++i)
		{
			std::cout << (availabilities[x * 32 + i] ? 1 : 0);
		}
		std::cout << std::endl;
	}*/
}

std::string ShaderFunctionModel::GenerateFunctionDeclaration()
{
	std::string funcDecl;

	switch (funcType)
	{
	case Primitive:
		funcDecl = "float " + name + "( in vec3 point";
		break;
	case SpaceRemap:
		funcDecl = "mat4 " + name + "( in mat4 m, in vec3 point";
		break;
	}

	for (AttribModel parameter : parameters)
	{
		funcDecl += ", " + ToString(parameter.type) + " " + parameter.name;
	}

	funcDecl += ")\n{\n" + functionBody + "}\n\n";

	return funcDecl;
}

void ShaderFunctionModel::Save(std::string path, std::string fileName, void* data /* useless but needed to be compatible with a delegate*/)
{
	std::ofstream file;
	std::string saveString;
	sourcePath = path;
	sourceName = fileName;
	file.open(sourcePath + sourceName);

	if (file.is_open())
	{
		saveString += "type " + ToString(funcType) + "\n";
		saveString += "name " + name + "\n";

		for (AttribModel& attrib : parameters)
		{
			attrib.Save(saveString);
		}

		saveString += "body \n";
		saveString += functionBody;

		file.write(saveString.c_str(), saveString.size());
	}
}

ShaderFunctionModel* ShaderFunctionModel::Load(std::string path, std::string fileName)
{
	std::ifstream file;
	file.open(path + fileName);
	char buf[512];
	if (file.is_open())
	{
		ShaderFunctionModel* model = nullptr;
		bool finished = false;
		AttribModel* lastAttrib = nullptr;
		bool readingBody = false;
		while (!finished)
		{
			file.getline(buf, 512, '\n');
			std::string line = buf;

			if (!readingBody)
			{
				std::string word = std::string(line.c_str(), line.find_first_of(' '));
				std::string value = std::string(line.c_str() + line.find_first_of(' ') + 1);

				if (word == "type")
				{
					model = new ShaderFunctionModel(ToFunctionType(value));
					model->sourcePath = path;
					model->sourceName = fileName;
				}
				else if (model == nullptr) return nullptr;
				else if (word == "name")
				{
					model->name = value;
				}
				else if (word == "attribName")
				{
					model->parameters.push_back(AttribModel());
					lastAttrib = &model->parameters.back();
					lastAttrib->name = value;
				}
				else if (word == "attribType")
				{
					lastAttrib->type = ToAttribType(value);
					lastAttrib->oldType = lastAttrib->type;
				}
				else if (word == "attribDesc")
				{
					lastAttrib->type = ToAttribType(value);
					lastAttrib->oldType = lastAttrib->type;

					for (int i = 0; i < 4; ++i)
					{
						file.getline(buf, 512, '\n');
						std::string line = buf;
						std::string word = std::string(line.c_str(), line.find_first_of(' '));
						std::string value = std::string(line.c_str() + line.find_first_of(' ') + 1);

						*((&lastAttrib->hasMin) + i) = std::stoi(word);
						*((&lastAttrib->minimum) + i) = std::stof(value);
					}
				}
				else if (word == "body")
				{
					model->functionBody = "";
					readingBody = true;
				}
			}
			else
			{
				model->functionBody += line + "\n";
			}

			if (file.eof())
			{
				break;
			}
		}
		return model;
	}
	return nullptr;
}

ShaderFunctionModel::ShaderFunctionModel(FunctionType _funcType) : funcType(_funcType)
{
	switch (funcType)
	{
	case Primitive:
		name = "newPrimitive";
		functionBody = "return length(point) - 0.5;\n";
		break;
	case SpaceRemap:
		name = "newSpaceRemap";
		functionBody =
			"vec3 surfP = clamp(point, -0.5, 0.5);\n"
			"vec3 diff = point - surfP;\n"
			"vec3 nor = abs(normalize(diff));\n"
			"int dim = nor.x > nor.z ? (nor.x > nor.y ? 0 : 1) : (nor.z > nor.y ? 2 : 1);\n"
			"int dimM = dim - 1;\n"
			"dimM = dimM < 0 ? 3 + dimM : dimM;\n"
			"int dimP = dim + 1;\n"
			"dimP = dimP > 2 ? 3 - dimP : dimP;\n"

			"vec2 rotOffset = vec2(atan(diff[dimP], abs(diff[dim])), atan(diff[dimM], abs(diff[dim])))\n"
			"/ 3.14159265 * 0.1 * 3.0;\n"
			"vec2 uv = mod(vec2(surfP[dimM], surfP[dimP]) + vec2(rotOffset.y, rotOffset.x) + 0.5 * vec2(0.5), vec2(0.5)) - 0.5f * vec2(0.5);\n"
			"vec3 fakePoint = vec3(uv.x, (sdBox(point, vec3(0.5))) - 0.1, uv.y);\n"
			"return InvTranslate(m, point - fakePoint);";
		break;
	}

	ID = ShaderFunction::models[funcType].size();
	ShaderFunction::models[funcType].push_back(this);
}

ShaderFunctionModel::~ShaderFunctionModel()
{
	onModelDeleted();
	ShaderFunction::models[funcType].remove(this);
}

// PRIMITIVE //

ShaderFunction::ShaderFunction(ShaderFunctionModel& _model) : model(_model)
{
	model.onModelDeleted.Bind(this, &ShaderFunction::ModelWillDestroy);
	model.onModelChanged.Bind(this, &ShaderFunction::UpdateFunction);
	UpdateFunction();
}

ShaderFunction::ShaderFunction(const ShaderFunction& _func) : ShaderFunction(_func.model)
{
	for (int i = 0; i < attribs.size(); ++i)
	{
		for (int j = 0; j <= attribs[i].model.type; ++j)
		{
			attribs[i].val[j] = _func.attribs[i].val[j];
		}
	}

	SetDynamic(_func.IsDynamic());
}

ShaderFunction::~ShaderFunction()
{
	SetDynamic(false);
	model.onModelDeleted.Unbind(this, &ShaderFunction::ModelWillDestroy);
	model.onModelChanged.Unbind(this, &ShaderFunction::UpdateFunction);
}

void ShaderFunction::ReleaseDynamic(ShaderFunction* prim)
{
	if (!prim) return;
	for (auto it = prim->attribs.begin(); it != prim->attribs.end(); ++it)
	{
		Attrib& attrib = *it;
		SignOutAttrib(attrib);
	}
}

bool ShaderFunction::RequestDynamic(ShaderFunction* prim)
{
	if (!prim) return false;

	bool success = true;

	for (auto it = prim->attribs.begin(); it != prim->attribs.end(); ++it)
	{
		Attrib& attrib = *it;
		int foundStart = -1;
		bool foundPlace = false;

		for (int i = 0; i < CUSTOM_DATA_NUM; ++i)
		{
			if (availabilities[i])
			{
				if (foundStart < 0) foundStart = i;
				if (i - foundStart == attrib.model.type)
				{
					foundPlace = true;
					break;
				}
			}
			else
			{
				foundStart = -1;
			}
		}

		if (foundPlace)
		{

			SignInAttrib(attrib, foundStart);
		}
		else
		{
			success = false;
			break;
		}
	}

	if (!success)
	{
		ReleaseDynamic(prim);
	}

	return success;
}

void ShaderFunction::SignInAttrib(Attrib& attrib, int startID)
{
	if (startID >= 0)
	{
		float* tmpVal = &customData[startID];
		for (int i = 0; i <= attrib.model.type; ++i)
		{
			availabilities[startID + i] = false;
			tmpVal[i] = attrib.val[i];
		}

		if (attrib.startIndex < 0) delete attrib.val;
		attrib.val = tmpVal;
		attrib.startIndex = startID;
	}
}

void ShaderFunction::SignOutAttrib(Attrib& attrib)
{
	if (attrib.startIndex >= 0)
	{
		AttribType type = attrib.model.type != attrib.model.oldType ? attrib.model.oldType : attrib.model.type;
		float* tmpVal = new float[type + 1];
		for (int i = 0; i <= type; ++i)
		{
			availabilities[attrib.startIndex + i] = true;
			tmpVal[i] = attrib.val[i];
		}
		attrib.startIndex = -1;
		attrib.val = tmpVal;
	}
}

ShaderFunctionModel* ShaderFunction::GetModel(std::string sourcePath)
{
	std::string fileName = UIResourceBrowser::GetFileNameFromPath(sourcePath);
	for (int i = 0; i < FunctionType::FT_Count; ++i)
	{
		for (ShaderFunctionModel*& model : models[i])
		{
			if (model->sourceName == fileName)
			{
				return model;
			}
		}
	}

	return ShaderFunctionModel::Load(ShaderGenerator::GetProjectPath() + UIResourceBrowser::RemoveFileNameFromPath(sourcePath), fileName);
}

bool ShaderFunction::SetDynamic(bool dynamic)
{
	if (dynamic == IsDynamic()) return true;

	if (dynamic)
	{
		return RequestDynamic(this);
	}
	else
	{
		ReleaseDynamic(this);
	}

	return true;
}

Attrib::Attrib(AttribModel& _model) : model(_model)
{
	startIndex = -1;
	val = new float[model.type + 1];

	for (int i = 0; i < model.type + 1; ++i)
		if (model.hasDef) val[i] = model.defaultVal;
		else val[i] = 0;
}

Attrib::Attrib(const Attrib& _model) : model(_model.model)
{
	startIndex = -1;
	val = new float[model.type + 1];

	for (int i = 0; i < model.type + 1; ++i)
		val[i] = _model.val[i];
}

Attrib::~Attrib()
{
	if (startIndex != -1)
	{
		ShaderFunction::SignOutAttrib(*this);
	}

	delete[] val;
	val = nullptr;
}

std::string Attrib::WriteVal(int i)
{
	if (startIndex >= 0)
	{
		return "customData[" + std::to_string(startIndex + i) + "]";
	}
	else
	{
		return std::to_string(val[i]);
	}
}

std::string Attrib::GetValStr()
{
	std::string valStr;
	for (int i = 0; i <= model.type; ++i)
	{
		valStr += std::to_string(val[i]) + " ";
	}
	return valStr;
}

void Attrib::Load(std::string& inSave)
{
	std::string value;
	std::string rest = inSave;

	for (int i = 0; i <= model.type; ++i)
	{
		value = std::string(rest.c_str(), rest.find_first_of(' '));
		rest = std::string(rest.c_str() + rest.find_first_of(' ') + 1);

		val[i] = std::stof(value);
	}
}

void ShaderFunction::ModelWillDestroy()
{
	OnModelWillDestroy(this);
}

void ShaderFunction::UpdateFunction()
{
	bool dyn = IsDynamic() && attribs.size() > 0;

	std::map<std::string, Attrib> tmp;
	for (auto it = attribs.begin(); it != attribs.end(); ++it)
	{
		auto tmpPair = std::pair<std::string, Attrib>((*it).model.name, (*it));
		tmp.insert(tmp.begin(), tmpPair);
	}

	SetDynamic(false);
	attribs.clear();
	for (auto it = model.parameters.begin(); it != model.parameters.end(); ++it)
	{
		Attrib newAttrib(*it);
		auto tmpIt = tmp.find(newAttrib.model.name);
		if (tmpIt != tmp.end())
		{
			Attrib& old = (*tmpIt).second;
			int oldT = newAttrib.model.oldType;
			int newT = newAttrib.model.type;
			for (int i = 0; i <= newT; ++i)
			{
				if (i > oldT)
				{
					//newAttrib.val[i] = 0;
				}
				else
				{
					newAttrib.val[i] = old.val[i];
				}
			}
		}
		attribs.push_back(newAttrib);
	}
	SetDynamic(dyn);
}

std::string ShaderFunction::GenerateFunctionCall(int layer)
{
	std::string funcCall = model.name + "(";

	switch (model.funcType)
	{
	case FunctionType::SpaceRemap:
		funcCall += "m" + std::to_string(layer) + ", ";
	case FunctionType::Primitive:
		funcCall += "tmpP" + std::to_string(layer);
		break;
	}

	for (auto it = attribs.begin(); it != attribs.end(); ++it)
	{
		Attrib* attrib = &*it;
		funcCall += ", " + ToString(attrib->model.type) + "(" + attrib->WriteVal(0);

		for (int i = 1; i <= attrib->model.type; ++i)
		{
			funcCall += ", " + attrib->WriteVal(i);
		}

		funcCall += ")";
	}

	funcCall += ")";

	return funcCall;
}

void ShaderFunction::Save(std::string& outSave)
{
	outSave += "function " + UIResourceBrowser::GetRelativePath(model.sourcePath + model.sourceName, ShaderGenerator::GetProjectPath()) + "\n";
	outSave += "dynamic " + std::to_string(IsDynamic()) + "\n";

	for (Attrib& attrib : attribs)
	{
		outSave += "attribValue " + attrib.GetValStr() + "\n";
	}
}

void ShaderFunction::Load(std::stringstream& inSave)
{
	char buf[512];
	bool finished = false;
	bool dyn = false;
	auto attrib = attribs.begin();
	if (attrib != attribs.end())
	while (!finished)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		std::string word = std::string(line.c_str(), line.find_first_of(' '));
		std::string value = std::string(line.c_str() + line.find_first_of(' ') + 1);

		if (word == "dynamic")
		{
			dyn = (bool)std::stoi(value);
		}
		else
		{
			attrib->Load(value);
			attrib++;
			if (attrib == attribs.end()) finished = true;
		}
	}
	SetDynamic(dyn);
}

std::string ShaderFunction::GenerateFunctionsDeclaration()
{
	std::string funcDecl;

	for (int t = 0; t < FunctionType::FT_Count; ++t)
	{
		for (ShaderFunctionModel* model : models[t])
		{
			funcDecl += model->GenerateFunctionDeclaration();
		}
	}

	return funcDecl;
}

std::string ToString(AttribType attribType)
{
	switch (attribType)
	{
	case AttribType::Float:
		return std::string("float");
		break;
	case AttribType::Vec2:
		return std::string("vec2");
		break;
	case AttribType::Vec3:
		return std::string("vec3");
		break;
	case AttribType::Vec4:
		return std::string("vec4");
		break;
	default:
		return std::string("ERROR");
		break;
	}
}

std::string ToString(FunctionType funcType)
{
	switch (funcType)
	{
	case FunctionType::SpaceRemap:
		return std::string("SpaceRemap");
		break;
	case FunctionType::Primitive:
		return std::string("Primitive");
		break;
	default:
		return std::string("ERROR");
		break;
	}
}

AttribType ToAttribType(std::string attribType)
{
	if (attribType == "float")
		return AttribType::Float;
	else if (attribType == "vec2")
		return AttribType::Vec2;
	else if (attribType == "vec3")
		return AttribType::Vec3;
	else if (attribType == "vec4")
		return AttribType::Vec4;
}

FunctionType ToFunctionType(std::string funcType)
{
	if (funcType == "SpaceRemap")
		return FunctionType::SpaceRemap;
	else if (funcType == "Primitive")
		return FunctionType::Primitive;
}

void AttribModel::Save(std::string& saveString)
{
	saveString += "attribName " + name + "\n";

	saveString += "attribDesc " + ToString(type) + "\n";
	saveString += std::to_string(hasMin) + " " + std::to_string(minimum) + "\n";
	saveString += std::to_string(hasDef) + " " + std::to_string(defaultVal) + "\n";
	saveString += std::to_string(hasMax) + " " + std::to_string(maximum) + "\n";
	saveString += std::to_string(hasStep) + " " + std::to_string(step) + "\n";
	//saveString += "attribType " + ToString(type) + "\n"; // old save
}
