#include "Enemy.hpp"
#include "CommandQueue.hpp"
#include "Timer.hpp"
#include "math_utilities.h"
#include "GameWorld.hpp"

#define MAX_DISTANCE 1.5f
#define ROTATION_ANGLE 120.0f
#define MAX_ROTATION_TIME 0.5f
Enemy::Enemy(const std::string &profile,
					const std::string &objectName,
					const std::string &modelName,
					const SQTTransform &transform /*= SQTTransform()*/
)
:Character(profile, objectName, modelName, transform), m_State(State::RUNNING), m_DistanceCovered(0), m_rotationTime(0)
{
	m_LastPosition = getTransform().getPosition();
}

void Enemy::update()
{
	if(GameWorld::get().m_PlayIntro)
	{

	}
	else
	{ // AI
		if(getCurrentAnim()->m_Name != "lie")
		{
			CommandQueue::get().addCommandDisposable(new CommandCharacterRun(this->m_Name, Direction::FORWARD));
			glm::vec3 currPosition = getTransform().getPosition();
			float distanceOffset = glm::length(currPosition - m_LastPosition);
			m_DistanceCovered += distanceOffset;
			m_LastPosition = currPosition;

			if(m_State == State::RUNNING)
			{
				if(m_DistanceCovered >= MAX_DISTANCE)
				{
					m_State = State::ROTATING;
					m_rotationTime = 0.0f;
					m_DistanceCovered = 0.0f;
					m_Rotation = randomNumber(-ROTATION_ANGLE,ROTATION_ANGLE);
				}

			}
			else
			{
				float deltaTime = Timer::get().getLastInterval();
				if(m_rotationTime > MAX_ROTATION_TIME)
				{
					m_State = State::RUNNING;
				}
				else
				{
					m_Transform.pivotOnLocalAxisDegrees(0, m_Rotation * (deltaTime), 0);	
					m_rotationTime += deltaTime;
				}

			}
		}
	}

	Character::update();
}