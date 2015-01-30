#pragma once
#include "GameObject.hpp"
#include "Curve.hpp"
#include "ShaderManager.hpp"
#include "ShaderProgram.hpp"
#include "Mesh.hpp"
#include <luapath\luapath.hpp>
#include <string>
#include <GL/glew.h>

class Object;
class Character;

struct Attachment
{
	Object* m_Object;
	glm::vec3 m_OffsetPos;
	glm::vec3 m_OffsetRot;
	const Bone *m_BoneAttachment;
	const Bone *m_BoneIk;
	SkinnedObject *m_Parent;
	float m_Speed;
	CubicCurve m_Curve;
	float m_TimeExpired;
	bool m_Play;
	glm::quat m_SavedWeaponsRotation;//!< saved rotation before swing
	float m_DeltaWeaponOffset; //!< increment amount
	float m_TotalWeaponOffset; //!< total amount
	void updateAttachment();
	void loadAttachment(const luapath::Table &table);

};

struct HealthBar
{
	Character *m_Parent;
	float m_Yoffset;
	float m_Width;
	float m_Height;
	float m_Breadth;
	float m_HealthLeft; // 0.0 to 1.0
	float m_HitTimeDelay;
	glm::vec4 m_GoodColor, m_BadColor;
	float m_LastHitTime;
	Mesh m_BoxMesh;
	const ShaderProgram *m_Shader;
	//glm::vec3 m_GoodColor, m_BadColor;
	HealthBar();
	HealthBar(Character *parent, float yoffset, float width, float height, float breadth);
	void load(Character *parent);
	void updateHealth(float factor);
	void render();
};

/**
@brief Represents a movable biped 
*/
class Character
	:public SkinnedObject
{
public:
	Character(const std::string &profile,
		const std::string &objectName,
		const std::string &modelName,
		const SQTTransform &transform = SQTTransform());
	virtual ~Character();
	virtual void render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix);
	virtual void update();
	void playPrimary();
	void playSecondary();
	virtual bool intersect(const AABB &other);
protected:
public:
	HealthBar m_HealthBar;
	Attachment m_Primary;
	Attachment m_Secondary; 
protected:

};