//#include "stdafx.h"
#include "ShaderManager.hpp"
#include <luapath\luapath.hpp>


using std::string;
using std::vector;
ShaderManager& ShaderManager::get()
{
	static ShaderManager singleton;
	return singleton;
}

ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{
	shaderProgramSet::iterator it = loadedShaderPrograms.begin();
	for (; it != loadedShaderPrograms.end(); ++it)
		delete *it;
}

ShaderProgram* ShaderManager::getShaderProgram(const std::string &vertexShaderName, const std::string &fragmentShaderName)
{

	ShaderProgram *newShader = new ShaderProgram(ShaderProgram::Key(vertexShaderName, fragmentShaderName));
	shaderProgramSet::const_iterator it =
		std::find_if(loadedShaderPrograms.begin(), loadedShaderPrograms.end(),
		[newShader](const ShaderProgram* other) -> bool { return *newShader == *other; });

	if (it == loadedShaderPrograms.end())
	{
		//construct new shaders
		//load settings file to find out info on the shaders
		luapath::LuaState settings("config/settings.lua");

		//note the dot '.' ; recall that luapath uses '.' to traverse the next string key and '#' next integer key
		string vsFullPath = settings.getGlobalTable("vertexShaders").getValue(string(".") + vertexShaderName);

		luapath::Table currFSShader = settings.getGlobalTable("fragmentShaders").getTable(string(".")+fragmentShaderName);
		addSamplers(currFSShader,newShader);
		string fsFullPath = currFSShader.getValue(".shaderDir");
		GLuint programId = createProgram(vsFullPath.c_str(), fsFullPath.c_str());
		newShader->m_Id = programId;

		loadedShaderPrograms.insert(newShader);
		return newShader;
	}
	else{
		delete newShader;
		//return existing program with those names
		return *it;
	}
	
}

void ShaderManager::addSamplers(const luapath::Table &shaderTable, ShaderProgram *currShader)
{
	currShader->samplers["diffuse"] = vector<string>();
	currShader->samplers["specular"] = vector<string>();
	currShader->samplers["ambient"] = vector<string>();
	currShader->samplers["shininess"] = vector<string>();

	luapath::Table samplerTable;
	if (shaderTable.getTable(".samplers", samplerTable))
	{
		luapath::Table samplerValues;
		if (samplerTable.getTable(".diffuse", samplerValues))
		{
			currShader->samplers.at("diffuse") = samplerValues.toArray<string>();
		}

		if (samplerTable.getTable(".specular", samplerValues))
		{
			currShader->samplers.at("specular") = samplerValues.toArray<string>();

		}
		if (samplerTable.getTable(".ambient", samplerValues))
		{
			currShader->samplers.at("ambient") = samplerValues.toArray<string>();

		}
		if (samplerTable.getTable(".shininess", samplerValues))
		{
			currShader->samplers.at("shininess") = samplerValues.toArray<string>();

		}
	}
}


GLuint ShaderManager::createProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	currProgramId = glCreateProgram();
	if (currProgramId == 0) {
		LOG(ERROR) << "Error creating shader program\n";
	}
	// Create two shader objects, one for the vertex, and one for the fragment shader
	tempVertexShaderId = addShader(vertexShaderSource, GL_VERTEX_SHADER);
	tempFragmentShaderId = addShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

	GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(currProgramId);
	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(currProgramId);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(currProgramId, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(currProgramId, sizeof(errorLog), NULL, errorLog);
		LOG(ERROR) << "Invalid shader program: " << errorLog << std::endl;
	}

	//cleanup
	glDetachShader(currProgramId, tempVertexShaderId);
	glDetachShader(currProgramId, tempFragmentShaderId);
	glDeleteShader(tempVertexShaderId);
	glDeleteShader(tempFragmentShaderId);

	return currProgramId;
}

GLuint ShaderManager::addShader(const char* shaderSource, GLenum shaderType)
{
	// create a shader object
	GLuint shaderId = glCreateShader(shaderType);

	if (shaderId == 0) {
		LOG(ERROR) << "Error creating shader type " << shaderType << std::endl;
	}
	char* shaderText = getFileContents(shaderSource);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(shaderId, 1, (const GLchar**)&shaderText, NULL);
	free(shaderText);
	// compile the shader and check for errors
	glCompileShader(shaderId);	
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024];
		glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
		if (shaderType == GL_VERTEX_SHADER)
			LOG(ERROR) << "Error compiling vertex shader: " << infoLog << std::endl;
		else if(shaderType == GL_FRAGMENT_SHADER)
			LOG(ERROR) << "Error compiling fragment shader: " << infoLog << std::endl;
	}
	// Attach the compiled shader object to the program object
	glAttachShader(currProgramId, shaderId);
	return shaderId;
}

char* ShaderManager::getFileContents(const char* filePath)
{
	FILE* fp;
	fopen_s(&fp,filePath, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}