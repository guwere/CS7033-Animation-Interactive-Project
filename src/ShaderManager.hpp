#pragma once
//#include "stdafx.h"
#include <GL/glew.h>
#include <unordered_set>

#include <luapath/luapath.hpp>
#include "ShaderProgram.hpp"


/**@brief A singleton class that manages the lifetime of ShaderProgam objects. 
@details Each pair(vertex shader, fragment shader) is considered a unique ShaderProgram.Loading of shaders should be done 
through this class. If a requested pair(vertex shader, fragment shader) has been loaded then the existing cached 
program is returned so duplication does not occur. A ShaderProgram is associated at Model level but different models
can use the same ShaderProgram.
*/
class ShaderManager
{
	
public:
	static ShaderManager& get();
	/**delete list of ShaderProgram */
	~ShaderManager();

	/**Load the shaders and return a pointer to the resulting program. May already be cached*/
	ShaderProgram* getShaderProgram(const std::string &vertexShaderName, const std::string &fragmentShaderName);

private:
	ShaderManager();
	GLuint addShader(const char* shaderSource, GLenum shaderType);
	char* getFileContents(const char* filePath);
	/**After the program has been loaded -- deletes the individual shader objects that were used to build program */
	GLuint createProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
	/**Retrieves the sampler names from the lua config file. The samplers that store texture object that is.*/
	void addSamplers(const luapath::Table &shaderTable, ShaderProgram *currShader);

private:
	typedef std::unordered_set<ShaderProgram*> shaderProgramSet;
	shaderProgramSet loadedShaderPrograms;

	GLuint currProgramId; //returned from createProgram
	//it is good practice to delete the shader after they have been linked
	GLuint tempVertexShaderId;
	GLuint tempFragmentShaderId;
	
};