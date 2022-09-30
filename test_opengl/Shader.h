#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Vector3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include "Delegate.h"


class Shader
{
public:

	Shader();
	Shader(const char* vertexPath, const char* fragmentPath);

	Notify onCompilation;

	void use() const;
	virtual bool Recompile(std::string& errorString);

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setIntArray(const std::string& name, unsigned int size, int* value) const;
	void setFloat(const std::string& name, float value) const;
	void setDouble(const std::string& name, double value) const;
	void setVector(const std::string& name, Vector3 value) const;
	void setVectorArray(const std::string& name, unsigned int size, Vector3* value) const;
	void setVectorArray(const std::string& name, unsigned int size, unsigned int dimension, float* value) const;
	void setFloatArray(const std::string& name, unsigned int size, float* value) const;
	void setMat4Array(const std::string& name, unsigned int size, glm::mat4* value) const;
	void setMat2x4Array(const std::string& name, unsigned int size, glm::mat2x4* value) const;
	std::string GetFragPath();

protected:

	unsigned int ID;
	unsigned int vertexShader;
	unsigned int fragmentShader;
	std::string vertexSrcPath;
	std::string fragmentSrcPath;
};

#endif