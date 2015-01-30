#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include <glm/gtc/type_ptr.hpp>

using std::vector;
using std::string;

Vertex::Vertex()
{
}

Vertex::Vertex(const glm::vec3 position)
	:m_Position(position)
{
}

void Vertex::operator=(const glm::vec3 &other)
{
	m_Position = other;
}

Mesh::Mesh()
{

}
Mesh::Mesh(const std::vector<Vertex> &vertices,
	const std::vector<GLuint> &indices,
	const std::vector<Texture> &textures,
	ShaderProgram *shader)
	:m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
{
	createVAO(shader);

	retrieveMaterialLocations(shader);
}

Mesh::~Mesh()
{
	destroy();
}

void Mesh::destroy()
{
	glDeleteBuffers(1,&m_VBO);
	glDeleteBuffers(1,&m_EBO);
	glDeleteVertexArrays(1,&m_VAO);
}
//code adapted from http://learnopengl.com/#!Model-Loading/Mesh
void Mesh::createVAO(const ShaderProgram *shader)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	if(m_Indices.size())
		glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

	if(m_Indices.size())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(GLuint), &m_Indices[0], GL_STATIC_DRAW);
	}

	// Set the vertex attribute pointers
	// Vertex Positions
	GLuint position = glGetAttribLocation(shader->m_Id, "position");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// Vertex Normals
	GLuint normal = glGetAttribLocation(shader->m_Id, "normal");
	glEnableVertexAttribArray(normal);
	glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Normal));
	// Vertex Texture Coords
	GLuint texCoord = glGetAttribLocation(shader->m_Id, "texCoord");
	glEnableVertexAttribArray(texCoord);
	glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_TexCoord));

	glBindVertexArray(0);
}

void Mesh::retrieveMaterialLocations(const ShaderProgram *shader)
{
	m_DiffuseLoc = glGetUniformLocation(shader->m_Id, "material.diffuse");
	m_SpecularLoc = glGetUniformLocation(shader->m_Id, "material.specular");
	m_AmbientLoc = glGetUniformLocation(shader->m_Id, "material.ambient");
	m_ShininessLoc = glGetUniformLocation(shader->m_Id, "material.shininess");
}

void Mesh::render(const ShaderProgram *shader, GLenum mode, bool drawElements) const
{
	//write material properties to gpu
	glUniform4fv(m_DiffuseLoc,1, glm::value_ptr(m_Material.diffuse)); 
	glUniform4fv(m_SpecularLoc,1, glm::value_ptr(m_Material.specular)); 
	glUniform4fv(m_AmbientLoc,1, glm::value_ptr(m_Material.ambient)); 
	glUniform1f(m_ShininessLoc, m_Material.shininess); 
	

	vector<string>::const_iterator diffuseTextures = shader->samplers.at("diffuse").begin();
	vector<string>::const_iterator specularTextures = shader->samplers.at("specular").begin();
	vector<string>::const_iterator shininessTextures = shader->samplers.at("shininess").begin();
	vector<string>::const_iterator ambientTextures = shader->samplers.at("ambient").begin();
	// Bind appropriate textures
	for (GLuint i = 0; i < m_Textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
		aiTextureType samplerType = m_Textures[i].m_Type;
		string textureName;
		if (samplerType == aiTextureType::aiTextureType_DIFFUSE)
		{
			textureName = *diffuseTextures++;
		}
		else if (samplerType == aiTextureType::aiTextureType_SPECULAR)
		{
			textureName = *specularTextures++;
		}
		else if (samplerType == aiTextureType::aiTextureType_SHININESS)
		{
			textureName = *shininessTextures++;
		}
		else if (samplerType == aiTextureType::aiTextureType_AMBIENT)
		{
			textureName = *ambientTextures++;
		}		// Now set the sampler to the correct texture unit number i
		glUniform1i(glGetUniformLocation(shader->m_Id, textureName.c_str()), i);
		// And finally bind the texture
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].m_Id);
	}
	glActiveTexture(GL_TEXTURE0); //  set everything back to defaults once configured.

	// Draw mesh
	glBindVertexArray(m_VAO);
	if(drawElements)
		glDrawElements(mode, m_Indices.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(mode, 0, m_Vertices.size());
	glBindVertexArray(0);
}

void Mesh::render(const ShaderProgram *shader) const
{
	render(shader,GL_TRIANGLES,true);
}

SkinnedVertex::SkinnedVertex(const Vertex &vertex)
{
	*this = vertex;
}
SkinnedVertex& SkinnedVertex::operator=(const Vertex &vertex)
{
	m_Position = vertex.m_Position;
	m_Normal = vertex.m_Normal;
	m_TexCoord = vertex.m_TexCoord;
	return *this;
}


SkinnedMesh::SkinnedMesh()
{
}

SkinnedMesh::SkinnedMesh(const std::vector<SkinnedVertex> &vertices,
	const std::vector<GLuint> &indices,
	const std::vector<Texture> &textures,
	ShaderProgram *shader)
	:m_SkinnedVertices(vertices)
{
	m_Indices = indices;
	m_Textures = textures;

	retrieveMaterialLocations(shader);
	createVAO(shader);
}


SkinnedMesh::~SkinnedMesh()
{

}
void SkinnedMesh::createVAO(ShaderProgram *shader)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_SkinnedVertices.size() * sizeof(SkinnedVertex), &m_SkinnedVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(GLuint), &m_Indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	GLuint position = glGetAttribLocation(shader->m_Id, "position");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (GLvoid*)0);
	// Vertex Normals
	GLuint normal = glGetAttribLocation(shader->m_Id, "normal");
	glEnableVertexAttribArray(normal);
	glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (GLvoid*)offsetof(SkinnedVertex, m_Normal));
	// Vertex Texture Coords
	GLuint texCoord = glGetAttribLocation(shader->m_Id, "texCoord");
	glEnableVertexAttribArray(texCoord);
	glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (GLvoid*)offsetof(SkinnedVertex, m_TexCoord));

	// Vertex Bone Indices. For now each vertex can be influenced by 4 bones
	//!@todo variable length bones
	GLuint boneIndices = glGetAttribLocation(shader->m_Id, "boneIndices");
	glEnableVertexAttribArray(boneIndices);
	glVertexAttribIPointer(boneIndices, 4, GL_UNSIGNED_INT, sizeof(SkinnedVertex), (GLvoid*)offsetof(SkinnedVertex, m_BoneIndices));

	// Vertex Bone Weights
	GLuint boneWeights = glGetAttribLocation(shader->m_Id, "boneWeights");
	glEnableVertexAttribArray(boneWeights);
	glVertexAttribPointer(boneWeights, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (GLvoid*)offsetof(SkinnedVertex, m_BoneWeights));


	glBindVertexArray(0);
}

void SkinnedMesh::render(const ShaderProgram *shader) const
{
	Mesh::render(shader);
}