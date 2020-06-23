#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Shader::Shader(const char* vertexName, const char* fragmentName)
{
	std::string vName(vertexName);
	std::string veName("res/" + vName + ".vert");
	const char* vertexPath = veName.c_str();
	std::string fName(fragmentName);
	std::string frName("res/" + fName + ".frag");
	const char* fragmentPath = (frName).c_str();

	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure& e) {
		std::cout << "Failed to read shader file \"" << vertexPath << "\" or \"" << fragmentPath << "\"! errno " << e.code() << std::endl;
		return;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();


	// compile
	GLuint vertex, fragment;
	int success;
	char infoLog[512];

	
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, nullptr);
	glCompileShader(vertex);
	
	// print compile errors
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		std::cout << "Failed to compile vertex shader. Error log:\n" << infoLog << std::endl;
		return;
	}
	
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, nullptr);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		std::cout << "Failed to compile fragment shader. Error log:\n" << infoLog << std::endl;
		return;
	}

	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);

	glLinkProgram(ID);
	// print linking errors if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, nullptr, infoLog);
		std::cout << "Failed to link shader! Error log:\n" << infoLog << std::endl;
		return;
	}

	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

// use/activate the shader
void Shader::use() const {
	glUseProgram(ID);
}

// utility uniform functions
void Shader::setBool(const std::string& name, const bool value) const {
	glUniform1i(getUniformLocation(name.c_str()), int(value));
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, const int value) const {
	glUniform1i(getUniformLocation(name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, const float value) const {
	glUniform1f(getUniformLocation(name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, const float x, const float y) const
{
	glUniform2f(getUniformLocation(name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const float x, const float y, const float z) const
{
	glUniform3f(getUniformLocation(name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const float x, const float y, const float z, const float w) const
{
	glUniform4f(getUniformLocation(name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

GLint Shader::getUniformLocation(const char* uniformName) const
{
	const auto f = uniformCache.find(uniformName);

	GLint loc;

	if (f == uniformCache.end()) {// get uniform location
		loc = glGetUniformLocation(ID, uniformName);

		std::pair<std::string, GLuint> newLoc(uniformName, loc);
		uniformCache.insert(newLoc);
	}
	else
	{
		loc = (*f).second;
	}

	return loc;
}

GLuint loadCompute(const char* computeShaderPath) {

	// Create the shaders
	const GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

	std::string computeShaderCode;
	std::ifstream computeShaderStream(computeShaderPath, std::ios::in);
	
	if (computeShaderStream.is_open()) {
		std::string line;
		while (getline(computeShaderStream, line))
			computeShaderCode += "\n" + line;
		computeShaderStream.close();
	}
	else {
		std::cout << "Failed to load compute shader " << computeShaderPath << std::endl;
		return 0;
	}

	GLint result = GL_FALSE;
	int infoLogLength;

	printf("Compiling shader : %s\n", computeShaderPath);
	char const* ComputeSourcePointer = computeShaderCode.c_str();
	glShaderSource(computeShader, 1, &ComputeSourcePointer, nullptr);
	glCompileShader(computeShader);

	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> ComputeShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(computeShader, infoLogLength, nullptr, &ComputeShaderErrorMessage[0]);
		printf("%s\n", &ComputeShaderErrorMessage[0]);
	}

	// Link the program
	const GLuint programID = glCreateProgram();
	glAttachShader(programID, computeShader);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(infoLogLength + 1);
		glGetProgramInfoLog(programID, infoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(programID, computeShader);

	glDeleteShader(computeShader);

	return programID;
}