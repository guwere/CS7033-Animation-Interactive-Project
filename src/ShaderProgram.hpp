#pragma once

#include "stdafx.h"
#include <GL/glew.h>

/**@brief Represents a loaded shader program on the gpu
@todo please encapsulate better. Look at m_Id variable. I don't see anywhere in the constructor
*/
struct ShaderProgram
{
public:
	struct Key{
		std::string vertexShader;
		std::string fragmentShader;
		Key(const std::string vertexShader, const std::string fragmentShader);
		bool operator==(const Key &other) const;
	};

	ShaderProgram(const ShaderProgram::Key &shaderNames);
	bool operator==(const ShaderProgram &other) const;
	bool operator==(const ShaderProgram::Key &other)const;

public:
	Key shaderNames;
	GLuint m_Id;
	//map key : diffuse, specular, etc. and map value : list of names for that type
	typedef std::map<std::string, std::vector<std::string> > SamplerMap;
	SamplerMap samplers;
};