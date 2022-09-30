#include "Shader.h"
#include "UIResourceBrowser.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	vertexSrcPath = vertexPath;
	fragmentSrcPath = fragmentPath;

	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vShaderFile.open(UIResourceBrowser::GetApplicationDirectory() + vertexPath);
		fShaderFile.open(UIResourceBrowser::GetApplicationDirectory() + fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();
		vertexCode = vShaderStream.str();
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
}

Shader::Shader()
{

}

void Shader::use() const
{
	glUseProgram(ID);
}

bool Shader::Recompile(std::string& errorString)
{
	int  success;
	char infoLog[512];
	std::string fragmentCode;
	std::ifstream fShaderFile;
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		fShaderFile.open(fragmentSrcPath);
		std::stringstream fShaderStream;
		fShaderStream << fShaderFile.rdbuf();
		fShaderFile.close();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* fShaderCode = fragmentCode.c_str();

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

	onCompilation();

	fragmentShader = tmpFragmentShader;
	ID = tmpID;
	return true;
}

void Shader::setBool(const std::string& name, bool value) const
{
	use();
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	use();
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setIntArray(const std::string& name, unsigned int size, int* value) const
{
	use();
	glUniform1iv(glGetUniformLocation(ID, name.c_str()), size, value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	use();
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setDouble(const std::string& name, double value) const
{
	use();
	glUniform1d(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVector(const std::string& name, Vector3 value) const
{
	use();
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}

void Shader::setVectorArray(const std::string& name, unsigned int size, Vector3* value) const
{
	use();
	float* vecTab = (float*)value;
	setVectorArray(name, size, 3, vecTab);
}

void Shader::setVectorArray(const std::string& name, unsigned int size, unsigned int dimension, float* value) const
{
	use();
	switch (dimension)
	{
	case 2:
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), size, value);
		break;
	case 3:
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), size, value);
		break;
	case 4:
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), size, value);
		break;
	}
}

void Shader::setFloatArray(const std::string& name, unsigned int size, float* value) const
{
	use();
	glUniform1fv(glGetUniformLocation(ID, name.c_str()), size, value);
}

void Shader::setMat4Array(const std::string& name, unsigned int size, glm::mat4* value) const
{
	use();
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), size, false, (float*)value);
}

void Shader::setMat2x4Array(const std::string& name, unsigned int size, glm::mat2x4* value) const
{
	use();
	glUniformMatrix2x4fv(glGetUniformLocation(ID, name.c_str()), size, false, (float*)value);
}

std::string Shader::GetFragPath()
{
	return std::string(fragmentSrcPath);
}
