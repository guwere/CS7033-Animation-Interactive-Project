#pragma once
#include "Mesh.hpp"
#include "stdafx.h"

class AABB
{
public:

	AABB();
	~AABB();
	void empty();
	void add(const glm::vec3 &point);
	void add(const std::vector<glm::vec3> points);
	void add(const std::vector<Vertex> vertices);
	void add(const std::vector<SkinnedVertex> vertices);
	bool intersect(const glm::vec3 &point);
	bool intersect(const AABB &aabb);
	bool enclose(const AABB &aabb);
	void render();
	void generateDisplayBox();
	void transform(const glm::mat4 &modelMatrix);
public:
	bool m_Enabled;
	bool m_Display;
	glm::vec3 m_Min;
	glm::vec3 m_Max;
	glm::vec3 m_InitialMin;
	glm::vec3 m_InitialMax;
	std::vector<glm::vec3> m_CubePoints;
	glm::vec3 m_LastPoint;
	std::vector<glm::vec3> m_InitialCubePoints;
	ShaderProgram *m_Shader;
	Mesh m_CubeMesh;
	glm::mat4 m_ModelMatrix;
private:

};