#include "Player.hpp"
#include "Character.hpp"
#include "Camera.hpp"
#include "GameWorld.hpp"
#include "Timer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <luapath\luapath.hpp>

using std::string; 
//#define CAMERA_POS glm::vec3(1.0f, 0.0f, 3.0f)
Player::Player()
	:m_CameraType(CameraType::FREE)
{
	//Camera *camera = new Camera(CAMERA_POS);
	luapath::LuaState settings("config/settings.lua");
	luapath::Table playerTable = settings.getGlobalTable("player");
	string profileName = playerTable.getValue(".profile");
	m_Character = new Character(profileName,"Player", playerTable.getValue(".model"));
	glm::vec3 playerPosition(playerTable.getValue(".position.x"),playerTable.getValue(".position.y"),playerTable.getValue(".position.z"));
	m_Character->getTransform().setPosition(playerPosition);
	GameWorld::get().addSkinnedObject(m_Character);

	m_MouseSpeed = playerTable.getValue(".mouseSpeed");
	m_EyeYOffset = glm::vec3(playerTable.getValue(".eyeYOffset"));
	m_EyeXOffset = glm::vec3(playerTable.getValue(".eyeXOffset"));
	m_TargetYOffset = glm::vec3(playerTable.getValue(".targetYOffset"));

	m_Character->generateAABB();
}

Player::~Player()
{
	//if(m_Camera)
	//	delete m_Camera;
	//if(m_Character)
	//	delete m_Character;
}

//void Player::setCharacter(Character *character)
//{
//	m_Character = character;
//}
//void Player::setCamera(Camera *camera)
//{
//	m_Camera = camera;
//}

Character* Player::getCharacter()
{
	return m_Character;
}
//Camera* Player::getCamera()
//{
//	return m_Camera;
//}

glm::mat4 Player::getViewMatrix()
{
	glm::vec3 forward = m_Character->getTransform().getForwardDirection();
	glm::vec3 position =  m_Character->getTransform().getPosition();
	glm::vec3 up = m_Character->getTransform().getUpDirection();
	glm::vec3 eye = position - forward*m_EyeXOffset + up*m_EyeYOffset;
	return glm::lookAt(eye, position + up*m_TargetYOffset,up);
}

void Player::mouseMove(double xPos, double yPos)
{

	float  xoffset = xPos - m_LastX;
    float yoffset = m_LastY - yPos;  // Reversed since y-coordinates go from bottom to left
	m_LastX = xPos;
	m_LastY = yPos;
	float deltaTime = Timer::get().getLastInterval();
	m_Character->getTransform().pivotOnLocalAxis(0, -deltaTime*m_MouseSpeed*xoffset, 0);
	m_EyeYOffset -= deltaTime*m_MouseSpeed*yoffset;
	//m_Character->getBoneTransform("neck").pivotOnLocalAxis(-deltaTime*m_MouseSpeed*yoffset, 0, 0);
}
