#include "Command.hpp"
#include "Player.hpp"
#include "Character.hpp"
#include "GameWorld.hpp"
#include "Camera.hpp"
#include "Timer.hpp"
#include "logger\Logger.hpp"

//animation speeds
#define LIE_SPEED 1.0f
#define ANIM_WAIT_SPEED 0.5f
#define ANIM_FORWARD_RUN_SPEED 1.0f
#define ANIM_BACKWARD_RUN_SPEED 1.5f
#define ANIM_STRAFE_RUN_SPEED 1.5f
#define ANIM_FORWARD_WALK_SPEED 1.0f
#define ANIM_BACKWARD_WALK_SPEED 1.5f
#define ANIM_STRAFE_WALK_SPEED 0.75f
//movement speeds (displacements)
#define MOVE_FORWARD_RUN_SPEED 1.0f
#define MOVE_BACKWARD_RUN_SPEED 0.25f
#define MOVE_STRAFE_RUN_SPEED 0.3f
#define MOVE_FORWARD_WALK_SPEED 1.0f
#define MOVE_BACKWARD_WALK_SPEED 0.5f
#define MOVE_STRAFE_WALK_SPEED 0.5f
//collision
#define COLLISION_DELAY 0.5f // the time between two consecutive collisions
#define COLLISION_ROTATION 180.0f // how much to rotate upon colliding with another object
//weapon
#define HEALTH_DECREASE 0.2f
CommandNoInput::CommandNoInput()
{
}
void CommandNoInput::execute()
{
	//Character *character = dynamic_cast<Character*>(GameWorld::get().getSkinnedObject("Player"));
	SkinnedObject *player = GameWorld::get().getSkinnedObject("Player");
	if(player->getCurrentAnim())
	{
		if(player->getCurrentAnim()->m_Name != "wait") // if not already waiting
			GameWorld::get().getSkinnedObject("Player")->playAnimBlend("wait", ANIM_WAIT_SPEED);	
	}
	else
		GameWorld::get().getSkinnedObject("Player")->playAnimBlend("wait", ANIM_WAIT_SPEED);	

	
}

CommandCharacterRun::CommandCharacterRun(std::string characterName, Direction direction)
	:m_Name(characterName), m_Direction(direction) 
{
}
void CommandCharacterRun::execute()
{
	float deltaTime = Timer::get().getLastInterval();
	Character *character = dynamic_cast<Character*>(GameWorld::get().getObject(m_Name));
	if(m_Direction != character->m_Direction)
	{
		character->m_Velocity = character->m_MinVelocity;
		character->m_Direction = m_Direction;
	}
	if(character->m_Velocity <= character->m_MaxRunVelocity)
		character->m_Velocity *= character->m_AccelerationRun;

	float offset = character->m_Velocity * deltaTime;

	switch (m_Direction)
	{
	case Direction::FORWARD:
	{
		character->getTransform().translateLocal(0, 0, offset*MOVE_FORWARD_RUN_SPEED);
		if(character->getCurrentAnim()->m_Name != "run" && character->getCurrentAnim()->m_Name != "lie")
			character->playAnimBlend("run", ANIM_FORWARD_RUN_SPEED * character->m_AnimationSpeedModifier);	
		break;
	}
	case Direction::BACKWARD:
	{
		character->getTransform().translateLocal(0, 0, -offset*MOVE_BACKWARD_RUN_SPEED);
		if(character->getCurrentAnim()->m_Name != "walkBackward")
			character->playAnimBlend("walkBackward", ANIM_BACKWARD_RUN_SPEED * character->m_AnimationSpeedModifier);	
		break;
	}
	case Direction::LEFT:
	{
		character->getTransform().translateLocal(offset*MOVE_STRAFE_RUN_SPEED, 0, 0);
		if(character->getCurrentAnim()->m_Name != "strafeLeft")
			character->playAnimBlend("strafeLeft", ANIM_STRAFE_RUN_SPEED * character->m_AnimationSpeedModifier);
		break;
	}
	case Direction::RIGHT:
		character->getTransform().translateLocal(-offset*MOVE_STRAFE_RUN_SPEED, 0, 0);
		if(character->getCurrentAnim()->m_Name != "strafeRight")
			character->playAnimBlend("strafeRight", ANIM_STRAFE_RUN_SPEED * character->m_AnimationSpeedModifier);
		break;
	}

}

CommandCharacterWalk::CommandCharacterWalk(std::string characterName, Direction direction)
	:m_Name(characterName), m_Direction(direction) 
{
}
void CommandCharacterWalk::execute()
{
	float deltaTime = Timer::get().getLastInterval();
	Character *character = dynamic_cast<Character*>(GameWorld::get().getSkinnedObject(m_Name));
	if(m_Direction != character->m_Direction)
	{
		character->m_Velocity = character->m_MinVelocity;
		character->m_Direction = m_Direction;
	}
	character->m_Velocity *= character->m_AccelerationWalk;
	if(character->m_Velocity >= character->m_MaxWalkVelocity)
		character->m_Velocity = character->m_MaxWalkVelocity;

	float offset = character->m_Velocity * deltaTime;

	switch (m_Direction)
	{
	case Direction::FORWARD:
	{
		character->getTransform().translateLocal(0, 0, offset*MOVE_FORWARD_WALK_SPEED);
		if(character->getCurrentAnim()->m_Name != "walk")
			character->playAnimBlend("walk",ANIM_FORWARD_WALK_SPEED * character->m_AnimationSpeedModifier);	
		break;
	}
	case Direction::BACKWARD:
	{
		character->getTransform().translateLocal(0, 0, -offset*MOVE_BACKWARD_WALK_SPEED);
		if(character->getCurrentAnim()->m_Name != "walkBackward")
			character->playAnimBlend("walkBackward",ANIM_BACKWARD_WALK_SPEED * character->m_AnimationSpeedModifier);	
		break;
	}
	case Direction::LEFT:
	{
		character->getTransform().translateLocal(offset*MOVE_STRAFE_WALK_SPEED, 0, 0);
		if(character->getCurrentAnim()->m_Name != "strafeLeft")
			character->playAnimBlend("strafeLeft",ANIM_STRAFE_WALK_SPEED * character->m_AnimationSpeedModifier);
		break;
	}
	case Direction::RIGHT:
		character->getTransform().translateLocal(-offset*MOVE_STRAFE_WALK_SPEED, 0, 0);
		if(character->getCurrentAnim()->m_Name != "strafeRight")
			character->playAnimBlend("strafeRight",ANIM_STRAFE_WALK_SPEED * character->m_AnimationSpeedModifier);
		break;
	}

}

CommandCharacterPrimary::CommandCharacterPrimary(std::string characterName)
	:m_Name(characterName)
{
}
void CommandCharacterPrimary::execute()
{
	//LOG(NORMAL) << "Primary";
	dynamic_cast<Character*>(GameWorld::get().getSkinnedObject(m_Name))->playPrimary();
}

CommandCharacterSecondary::CommandCharacterSecondary(std::string characterName)
	:m_Name(characterName)
{
}
void CommandCharacterSecondary::execute()
{
	LOG(NORMAL) << "Secondary";
}

CommandMouseMove::CommandMouseMove(double xPos, double yPos)
	:m_XPos(xPos), m_YPos(yPos)
{
}
void CommandMouseMove::execute()
{
	GameWorld::get().getPlayer().mouseMove(m_XPos, m_YPos);
}

CommandPlayAnim::CommandPlayAnim(std::string characterName, std::string animationName)
	:m_Name(characterName), m_AnimationName(animationName)
{
}
void CommandPlayAnim::execute()
{
	//GameWorld::get().getSkinnedObject(m_Name)->playAnimReset(m_AnimationName);
}

CommandLevelCollision::CommandLevelCollision(std::string characterName)
	:m_Name(characterName)
{

}
void CommandLevelCollision::execute()
{
	Object *object = GameWorld::get().getObject(m_Name);
	glm::vec3 direction = glm::normalize(object->getTransform().getPosition());
	object->getTransform().setPosition(glm::vec3(
		direction.x * GameWorld::get().m_LevelRadius,
		GameWorld::get().m_Level->getTransform().getPosition().y,
		direction.z * GameWorld::get().m_LevelRadius));

	if(object->m_Name != "Player")
	{
		object->getTransform().pivotOnLocalAxis(0,COLLISION_ROTATION, 0);
	}
}

CommandObjectCollision::CommandObjectCollision(Object *object1, Object *object2)

	:m_Object1(object1), m_Object2(object2)
{

}
void CommandObjectCollision::execute()
{
	float currTime = Timer::get().getTime();
	float collisionInterval1 = currTime - m_Object1->m_LastCollisionTime;
	if(m_Object1->m_Name != "Player" && m_Object1->m_Velocity > 0.0f && collisionInterval1 > COLLISION_DELAY)
	{
		glm::vec3 direction = m_Object1->m_AABB.m_CubePoints[0] - m_Object2->m_AABB.m_CubePoints[0];
		direction.y = 0;
		m_Object1->getTransform().translateLocal(-direction);
		m_Object1->getTransform().pivotOnLocalAxis(0.0f,COLLISION_ROTATION,0.0f);
		m_Object1->m_LastCollisionTime = currTime;
	}
	float collisionInterval2 = currTime - m_Object2->m_LastCollisionTime;
	if (m_Object2->m_Name != "Player" && m_Object2->m_Velocity > 0.0f && collisionInterval2 > COLLISION_DELAY)
	{
		glm::vec3 direction2 = m_Object2->m_AABB.m_CubePoints[0] - m_Object1->m_AABB.m_CubePoints[0];
		direction2.y = 0;
		m_Object2->getTransform().translateLocal(direction2);
		m_Object2->getTransform().pivotOnLocalAxis(0.0f,COLLISION_ROTATION,0.0f);
		m_Object2->m_LastCollisionTime = currTime;
	}
	LOG(INFO) << "obj1: " << m_Object1->m_Name << " obj2: " << m_Object2->m_Name;
}

CommandWeaponCollision::CommandWeaponCollision(Character *object1, Character *object2)

	:m_Object1(object1), m_Object2(object2)
{

}
void CommandWeaponCollision::execute()
{
	m_Object2->playAnimBlend("lie",LIE_SPEED);
	m_Object2->m_HealthBar.updateHealth(-HEALTH_DECREASE);
	LOG(INFO) << "obj1: " << m_Object1->m_Name << " obj2: " << m_Object2->m_Name;
}