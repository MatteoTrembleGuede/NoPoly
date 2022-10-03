#pragma once
#include <list>
#include <vector>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "Delegate.h"

#define CUSTOM_DATA_NUM 128

class Shader;

typedef enum FunctionType
{
	SpaceRemap,
	Primitive,
	FT_Count
}FunctionType;

std::string ToString(FunctionType funcType);

typedef enum AttribType
{
	Float,
	Vec2,
	Vec3,
	Vec4,
	AT_Count
}AttribType;

std::string ToString(AttribType attribType);

class ShaderFunction;


typedef struct AttribModel
{
	std::string name;
	AttribType type = AttribType::Float;
	AttribType oldType = AttribType::Float;
	bool hasMin = false; 
	bool hasDef = false; 
	bool hasMax = false; 
	bool hasStep = false; 
	float minimum = -FLT_MAX;
	float defaultVal = 0;
	float maximum = FLT_MAX;
	float step = 0.1;
	void Save(std::string& saveString);
}AttribModel;

typedef struct Attrib
{
	const AttribModel& model;
	int startIndex;
	float* val;
	Attrib(AttribModel& _model);
	Attrib(const Attrib& _model);
	Attrib() = delete;
	~Attrib();
	std::string WriteVal(int i);
	std::string GetValStr();
	void Load(std::string& inSave);
}Attrib;

typedef struct ShaderFunctionModel
{
	const FunctionType funcType;
	std::string name;
	std::string sourcePath;
	std::string sourceName;
	int ID;
	std::string functionBody;
	std::list<AttribModel> parameters;
	Notify onModelChanged;
	Notify onModelDeleted;

	std::string GenerateFunctionDeclaration();
	void Save(std::string path, std::string fileName, void* data);
	static ShaderFunctionModel* Load(std::string path, std::string fileName);

	ShaderFunctionModel(FunctionType _funcType);
	~ShaderFunctionModel();
}ShaderFunctionModel;

DeclareDelegate(ShaderFunctionDelegate, ShaderFunction*);

class ShaderFunction
{
public:

	ShaderFunctionModel& model;
	std::vector<Attrib> attribs;
	ShaderFunctionDelegate OnModelWillDestroy;

	ShaderFunction(const ShaderFunction& _func);
	ShaderFunction(ShaderFunctionModel& _model);
	~ShaderFunction();

	bool SetDynamic(bool dynamic);
	bool IsDynamic() const { return (attribs.size() == 0) || (attribs.front().startIndex >= 0); };
	std::string GenerateFunctionCall(int layer);
	void Save(std::string& outSave);
	void Load(std::stringstream& inSave);

private:

	void ModelWillDestroy();
	void UpdateFunction();

public:	// STATIC //

	static std::list<ShaderFunctionModel*> models[FunctionType::FT_Count];

	static std::string GenerateFunctionsDeclaration();
	static void SendData(Shader* shader);
	static void Init() { memset(availabilities, true, CUSTOM_DATA_NUM); };
	static void SignOutAttrib(Attrib& attrib);
	static ShaderFunctionModel* GetModel(std::string sourcePath);

private:

	static float customData[CUSTOM_DATA_NUM];
	static bool availabilities[CUSTOM_DATA_NUM];

	static void ReleaseDynamic(ShaderFunction* prim);
	static bool RequestDynamic(ShaderFunction* prim);
	static void SignInAttrib(Attrib& attrib, int startID);
};

