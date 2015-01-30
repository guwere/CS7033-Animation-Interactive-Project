#pragma once
#include <glm/glm.hpp>
class Character;
class Camera;
/**
@brief A singleton representing the player controllable entities i.e. a camera and a character
*/
class Player
{
public:
	enum class CameraType{CHARACTER,FREE}; //@todo
	/**@brief start with a default state as specified in the settings file*/
	Player();
	~Player();
	//void setCharacter(Character *character);
	//void setCamera(Camera *camera);
	Character* getCharacter();
	//Camera* getCamera();
	glm::mat4 getViewMatrix();
	void mouseMove(double xPos, double yPos);
public:
	glm::vec3 m_EyeYOffset;
	glm::vec3 m_EyeXOffset;
	glm::vec3 m_TargetYOffset;
private:
	CameraType m_CameraType;
	Character *m_Character;
	//Camera *m_Camera;
	float m_MouseSpeed;
	float m_LastX;
	float m_LastY;
};