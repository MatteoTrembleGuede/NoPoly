#include "ShaderGenerator.h"
#include "ShaderNode.h"
#include "ShaderLeaf.h"
#include "Lighting.h"
#include "Light.h"
#include "ViewportManager.h"
#include "Material.h"
#include "CustomPrimitive.h"
#include "UIResourceBrowser.h"

std::string ShaderGenerator::projectPath;

ShaderGenerator::ShaderGenerator()
{
	lighting = new Lighting(this);
	root = new ShaderNode();
	root->name = "scene name";
	vertexSrcPath = "";
	fragmentSrcPath = "";

	DistWithColorFuncStart = "float DistToScene(in vec3 p, in vec3 n, out vec3 color)\n{\n"
		"\tfloat minDist = 999999999;\n"
		"\tcolor = vec3(0, 0, 0);\n"
		"\tfloat tmpDist = 0; \n"
		"\tvec3 tmpP = p; \n"
		"\tvec3 tmpN = n; \n"
		"\tmat4 m = mat4(1, 0, 0, 0, \n"
			"\t\t0, 1, 0, 0, \n"
			"\t\t0, 0, 1, 0, \n"
			"\t\t0, 0, 0, 1);\n ";
	
	DistWithLightPropertiesFuncStart = "	return minDist;\n"
		"}\n\n"

		"float DistToScene(in vec3 p, out vec3 lp)\n{\n"
		"\tfloat minDist = 999999999;\n"
		"\tlp = vec3(0, 0, 0);\n"
		"\tfloat tmpDist = 0; \n"
		"\tvec3 tmpP = p; \n"
		"\tmat4 m = mat4(1, 0, 0, 0, \n"
			"\t\t0, 1, 0, 0, \n"
			"\t\t0, 0, 1, 0, \n"
			"\t\t0, 0, 0, 1);\n ";

	DistOnlyFuncStart = "	return minDist;\n"
	"}\n\n"

	"float DistToScene(in vec3 p)\n"
	"{\n"
	"\tfloat minDist = 999999999;\n"
	"\tfloat tmpDist = 0;\n"
	"\tvec3 tmpP = p;\n"
	"\tmat4 m = mat4(1, 0, 0, 0,\n"
		"\t\t0, 1, 0, 0,\n"
		"\t\t0, 0, 1, 0,\n"
		"\t\t0, 0, 0, 1); \n";

	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderStartFile;
	std::ifstream fShaderEndFile;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderStartFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderEndFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		std::stringstream vShaderStream, fShaderStream, tmp, tmp2, tmp3;

		// vertex shader loading
		vShaderFile.open(UIResourceBrowser::GetApplicationDirectory() + "Shaders/VertexShader.shader");
		vShaderStream << vShaderFile.rdbuf();
		vShaderFile.close();

		vertexCode = vShaderStream.str();

		// fragment shader loading
		fShaderStartFile.open(UIResourceBrowser::GetApplicationDirectory() + "Shaders/CompositeFragmentShaderStart.shaderPart");
		tmp << fShaderStartFile.rdbuf();
		HeadCode = tmp.str();
		fShaderStream << fShaderStartFile.rdbuf() << DistWithColorFuncStart << DistWithLightPropertiesFuncStart << DistOnlyFuncStart;
		fShaderStartFile.close();

		fShaderEndFile.open(UIResourceBrowser::GetApplicationDirectory() + "Shaders/CompositeFragmentShaderEnd.shaderPart");
		tmp3 << std::string("	return minDist;\n}\n\n") << fShaderEndFile.rdbuf();
		TailCode = tmp3.str();
		fShaderStream << TailCode;
		fShaderEndFile.close();

		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	projectPath = "";
}

ShaderGenerator::~ShaderGenerator()
{
	delete root;
}

bool ShaderGenerator::Recompile(std::string& errorString, std::string& errorCode)
{
	int  success;
	char infoLog[512];
	std::string fragmentCode;
	std::stringstream fShaderStream;
	fShaderStream << HeadCode << ShaderFunction::GenerateFunctionsDeclaration() << DistWithColorFuncStart;

	std::string generatedCode;
	root->GenerateCode(generatedCode, 0);
	fShaderStream << generatedCode;

	fShaderStream << DistWithLightPropertiesFuncStart;

	generatedCode.clear();
	root->GenerateCode(generatedCode, 0, LightingProp);
	fShaderStream << generatedCode;

	fShaderStream << DistOnlyFuncStart;

	generatedCode.clear();
	root->GenerateCode(generatedCode, 0, DistanceOnly);
	fShaderStream << generatedCode;

	fShaderStream << TailCode;

	fragmentCode = fShaderStream.str();
	const char* fShaderCode = fragmentCode.c_str();

	errorCode = fragmentCode;

	unsigned int tmpFragmentShader;
	tmpFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(tmpFragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(tmpFragmentShader);

	glGetShaderiv(tmpFragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(tmpFragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		glDeleteShader(tmpFragmentShader);
		errorString = infoLog;
		return false;
	}

	unsigned int tmpID = glCreateProgram();
	glAttachShader(tmpID, vertexShader);
	glAttachShader(tmpID, tmpFragmentShader);
	glLinkProgram(tmpID);

	glGetProgramiv(tmpID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(tmpID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;

		glDeleteShader(tmpFragmentShader);
		glDeleteProgram(tmpID);
		errorString = infoLog;
		return false;
	}

	glDeleteProgram(ID);
	glDeleteShader(fragmentShader);

	fragmentShader = tmpFragmentShader;
	ID = tmpID;
	ViewportManager::AdjustQuad();

	Material::SendData(this);

	onCompilation();

	return true;
}

void ShaderGenerator::Clear()
{
	delete root;
	root = new ShaderNode();
	Light::Clear();
	lighting->Reset();
	Material::Clear();
	Material::SendData(this);
	ViewportManager::AdjustQuad();
	projectPath = "";

	fileError = false;
}

bool ShaderGenerator::Save(std::string& path, std::string& errorString, ShaderNode* rootPart)
{
	if (rootPart == nullptr) rootPart = root;
	std::ofstream saveFile;
	saveFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	try
	{
		std::string saveString;
		projectPath = UIResourceBrowser::RemoveFileNameFromPath(path);
		saveFile.open(path);
		rootPart->Save(saveString);
		lighting->Save(saveString);
		Material::Save(saveString);
		saveFile.write(saveString.c_str(), saveString.size());
		saveFile.close();
	}
	catch (std::ofstream::failure e)
	{
		fileError = true;
		errorString = "couldn't open/create file : " + path;
		return false;
	}

	fileError = false;
	return true;
}

bool ShaderGenerator::Load(std::string& path, std::string& errorString, ShaderNode** rootPart)
{
	if (rootPart == nullptr) rootPart = &root;
	std::ifstream saveFile;
	saveFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		std::stringstream saveStream;
		ShaderPart* parent = (*rootPart)->GetParent();
		ShaderNode* old = (*rootPart);
		projectPath = UIResourceBrowser::RemoveFileNameFromPath(path);
		// vertex shader loading
		saveFile.open(path);
		saveStream << saveFile.rdbuf();
		saveFile.close();
		(*rootPart) = new ShaderNode();
		char tmp[512];
		saveStream.getline(tmp, 512, '\n');
		(*rootPart)->Load(saveStream);
		(*rootPart)->SetParent(parent);
		if (parent)
		{
			auto it = ((ShaderNode*)parent)->parts.begin();

			for (; it != ((ShaderNode*)parent)->parts.end(); ++it)
			{
				if ((*it) == old) break;
			}

			((ShaderNode*)parent)->parts.insert(it, 1, (*rootPart));
			((ShaderNode*)parent)->parts.remove(old);
		}
		else
		{
			Light::Clear();
		}
		std::swap((*rootPart), old);
		(*rootPart) = old;
		if (!saveStream.eof())
		{
			lighting->Load(saveStream);
		}
		Material::Load(saveStream);
		Material::SendData(this);
	}
	catch (std::ofstream::failure e)
	{
		fileError = true;
		errorString = "couldn't open file : " + path;
		return false;
	}

	ViewportManager::AdjustQuad();

	fileError = false;
	return true;
}

void ShaderGenerator::InsertGroupBefore(ShaderPart*& part)
{
	ShaderNode* newNode = new ShaderNode();
	ShaderPart* parent = part->GetParent();
	part->SetParent(newNode);
	newNode->SetParent(parent);
	newNode->parts.push_back(part);
	ShaderPart* newNodeAsPart = (ShaderPart*)newNode;
	std::swap(part, newNodeAsPart);
}

void ShaderGenerator::InsertGroupBefore(ShaderNode*& part)
{
	ShaderNode* newNode = new ShaderNode();
	ShaderPart* parent = part->GetParent();
	part->SetParent(newNode);
	newNode->SetParent(parent);
	newNode->parts.push_back(part);
	std::swap(part, newNode);
}

std::string ShaderGenerator::GetProjectPath()
{
	return (projectPath.size() > 0 ? projectPath : UIResourceBrowser::GetWorkingDirectory());
}
