#include "ShaderProgram.hpp"

ShaderProgram::Key::Key(const std::string vertexShader, const std::string fragmentShader)
	:vertexShader(vertexShader), fragmentShader(fragmentShader)
{

}

ShaderProgram::ShaderProgram(const ShaderProgram::Key &shaderNames)
	: shaderNames(shaderNames)
{

}

bool ShaderProgram::Key::operator==(const Key &other) const
{
	return this->vertexShader == other.vertexShader &&
		this->fragmentShader == other.fragmentShader;
}

bool ShaderProgram::operator==(const ShaderProgram &other) const
{
	return this->shaderNames == other.shaderNames;
}
bool ShaderProgram::operator==(const ShaderProgram::Key &other)const
{
	return this->shaderNames == other;
}