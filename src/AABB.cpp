#include "AABB.hpp"
#include "GameWorld.hpp"
#include "ShaderProgram.hpp"
#include "ShaderManager.hpp"
#include "math_utilities.h"

#include <luapath\luapath.hpp>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::vector;
using std::string;

AABB::AABB()
	:m_Display(false),m_Enabled(false)
{

}

void AABB::add(const std::vector<glm::vec3> points)
{
	m_Enabled = true;
	for (int i = 0; i < points.size(); i++)
	{
		add(points[i]);
	}
}

void AABB::add(const std::vector<Vertex> vertices)
{
	m_Enabled = true;
	for (int i = 0; i < vertices.size(); i++)
	{
		add(vertices[i].m_Position);
	}
}

void AABB::add(const std::vector<SkinnedVertex> vertices)
{
	m_Enabled = true;
	for (int i = 0; i < vertices.size(); i++)
	{
		add(vertices[i].m_Position);
	}
}

AABB::~AABB()
{
	if(m_CubeMesh.m_Vertices.size())
		m_CubeMesh.destroy();
}


void AABB::empty()
{
	m_Min = glm::vec3(std::numeric_limits<float>::max());
	m_Max = glm::vec3(std::numeric_limits<float>::min());
}

void AABB::add(const glm::vec3 &point)
{
	if(point.x < m_Min.x) m_Min.x = point.x;
	if(point.x > m_Max.x) m_Max.x = point.x;
	if(point.y < m_Min.y) m_Min.y = point.y;
	if(point.y > m_Max.y) m_Max.y = point.y;
	if(point.z < m_Min.z) m_Min.z = point.z;
	if(point.z > m_Max.z) m_Max.z = point.z;


}

bool AABB::intersect(const glm::vec3 &point)
{
	if (point.x > m_Max.x) return false;
	if (point.x < m_Min.x) return false;
	if (point.y > m_Max.y) return false;
	if (point.y < m_Min.y) return false;
	if (point.z > m_Max.z) return false;
	if (point.z < m_Min.z) return false;
	return true;
}
bool AABB::intersect(const AABB &aabb)
{

	for (int i = 0; i < 8; i++)
	{
		if(intersect(aabb.m_CubePoints[i]))
			return true;
	}
	return false;
}


bool AABB::enclose(const AABB &aabb)
{
	if (m_Max.x > aabb.m_Max.x) return false;
	if (m_Min.x < aabb.m_Min.x) return false;
	if (m_Max.y > aabb.m_Max.y) return false;
	if (m_Min.y < aabb.m_Min.y) return false;
	if (m_Max.z > aabb.m_Max.z) return false;
	if (m_Min.z < aabb.m_Max.z) return false;
	return true;
}

void AABB::transform(const glm::mat4 &modelMatrix)
{
	glm::vec3 translation(modelMatrix[3]);
	m_ModelMatrix = glm::translate(glm::mat4(), translation);
	m_Min = m_InitialMin + translation;
	m_Max = m_InitialMax + translation;
	for (int i = 0; i < m_CubePoints.size(); i++)
	{
		m_CubePoints[i] = glm::vec3(m_ModelMatrix * glm::vec4(m_InitialCubePoints[i],1.0f));
	}

}


void AABB::generateDisplayBox()
{
	luapath::LuaState settings("config/settings.lua");
	luapath::Table debugTable = settings.getGlobalTable("debug");
	luapath::Table aabbTable = debugTable.getTable(".aabb");
	bool isEnabled = aabbTable.getValue(".enable");

	//randomize the min and max just a tad bit for better response

	m_Min = glm::vec3(m_Min.x + randomNumber(-0.1f,0.1f),m_Min.y + randomNumber(-0.1f,0.1f), m_Min.z + randomNumber(-0.1f,0.1f));
	m_Max = glm::vec3(m_Max.x + randomNumber(-0.1f,0.1f),m_Max.y + randomNumber(-0.1f,0.1f), m_Max.z + randomNumber(-0.1f,0.1f));
	m_InitialMin = m_Min;
	m_InitialMax = m_Max;
	
	m_Display = isEnabled;
	m_CubePoints.resize(8);
	m_CubePoints[0] = m_Min;
	m_CubePoints[1] = glm::vec3(m_Max.x, m_Min.y, m_Min.z);
	m_CubePoints[2] = glm::vec3(m_Max.x, m_Min.y, m_Max.z);
	m_CubePoints[3] = glm::vec3(m_Min.x, m_Min.y, m_Max.z);
	m_CubePoints[4] = glm::vec3(m_Min.x, m_Max.y, m_Max.z);
	m_CubePoints[5] = glm::vec3(m_Min.x, m_Max.y, m_Min.z);
	m_CubePoints[6] = glm::vec3(m_Max.x, m_Max.y, m_Min.z);
	m_CubePoints[7] = m_Max;
	m_InitialCubePoints = m_CubePoints;
	m_LastPoint = m_CubePoints[0];

	if(!isEnabled)
		return;
	if(m_CubeMesh.m_Vertices.size())
		m_CubeMesh.destroy();

	vector<Vertex> cubeLines;
	cubeLines.push_back(Vertex(m_CubePoints[0]));
	cubeLines.push_back(Vertex(m_CubePoints[1]));
	
	cubeLines.push_back(Vertex(m_CubePoints[1]));
	cubeLines.push_back(Vertex(m_CubePoints[2]));
	
	cubeLines.push_back(Vertex(m_CubePoints[2]));
	cubeLines.push_back(Vertex(m_CubePoints[3]));
	
	cubeLines.push_back(Vertex(m_CubePoints[0]));
	cubeLines.push_back(Vertex(m_CubePoints[3]));
	
	cubeLines.push_back(Vertex(m_CubePoints[3]));
	cubeLines.push_back(Vertex(m_CubePoints[4]));
	
	cubeLines.push_back(Vertex(m_CubePoints[4]));
	cubeLines.push_back(Vertex(m_CubePoints[5]));

	cubeLines.push_back(Vertex(m_CubePoints[5]));
	cubeLines.push_back(Vertex(m_CubePoints[0]));
	
	cubeLines.push_back(Vertex(m_CubePoints[5]));
	cubeLines.push_back(Vertex(m_CubePoints[6]));
	
	cubeLines.push_back(Vertex(m_CubePoints[1]));
	cubeLines.push_back(Vertex(m_CubePoints[6]));
	
	cubeLines.push_back(Vertex(m_CubePoints[2]));
	cubeLines.push_back(Vertex(m_CubePoints[7]));
	
	cubeLines.push_back(Vertex(m_CubePoints[6]));
	cubeLines.push_back(Vertex(m_CubePoints[7]));
	
	cubeLines.push_back(Vertex(m_CubePoints[4]));
	cubeLines.push_back(Vertex(m_CubePoints[7]));

	m_CubeMesh.m_Vertices = cubeLines;
	m_CubeMesh.m_Material.diffuse = glm::vec4(aabbTable.getValue(".color.r"), aabbTable.getValue(".color.g"), aabbTable.getValue(".color.b"), aabbTable.getValue(".color.a"));

	m_Shader = ShaderManager::get().getShaderProgram(aabbTable.getValue(".vertexShader"),aabbTable.getValue(".fragmentShader"));
	m_CubeMesh.createVAO(m_Shader);
	m_CubeMesh.retrieveMaterialLocations(m_Shader);
}


void AABB::render()
{
	if(m_Display)
	{
		glUseProgram(m_Shader->m_Id);
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_ModelMatrix)); 
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(GameWorld::get().getViewMatrix())); 
		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(GameWorld::get().getProjectionMatrix()));
		m_CubeMesh.render(m_Shader, GL_LINES, false);
	}
}


