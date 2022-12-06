#pragma once
#include "Shader.h"

class Lighting;
class ShaderNode;
class ShaderLeaf;
class ShaderPart;

class ShaderGenerator :
    public Shader
{

public:
	Lighting* lighting;
	ShaderNode* root;
	bool fileError;

	ShaderGenerator();
	~ShaderGenerator();

	virtual bool Recompile(std::string& errorString, std::string& errorCode);
	void Clear();
	bool Save(std::string& path, std::string& errorString, ShaderNode* rootPart = nullptr);
	bool Load(std::string& path, std::string& errorString, ShaderNode** rootPart = nullptr);
	void InsertGroupBefore(ShaderPart*& part);
	void InsertGroupBefore(ShaderNode*& part);

	static std::string GetProjectPath();

protected:


private:

	std::string HeadCode;
	std::string ClosestBoundStart;
	std::string DistWithLightPropertiesFuncStart;
	std::string DistWithColorFuncStart;
	std::string DistOnlyFuncStart;
	std::string TailCode;

	static std::string projectPath;

};

