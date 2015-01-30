#pragma once
#include "Character.hpp"
#include "stdafx.h"
#include <glm/glm.hpp>

class Enemy
	: public Character
{
public:
	enum class State{RUNNING, ROTATING};
	Enemy(const std::string &profile,
		const std::string &objectName,
		const std::string &modelName,
		const SQTTransform &transform = SQTTransform());
	virtual void update();
public:
	State m_State;
	float m_DistanceCovered;
	glm::vec3 m_LastPosition;
	float m_rotationTime;
	float m_Rotation;
private:

};
