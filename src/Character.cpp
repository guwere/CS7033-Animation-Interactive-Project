#include "Character.hpp"
#include "Model.hpp"
#include "ModelManager.hpp"
#include "GameWorld.hpp"
#include "Timer.hpp"

#include <luapath\luapath.hpp>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::string;
using std::vector;
Character::Character(const std::string &profile,
					const std::string &objectName,
					const std::string &modelName,
					const SQTTransform &transform /*= SQTTransform()*/
)
:SkinnedObject(objectName, modelName, transform)
{
	luapath::LuaState settings("config/settings.lua");
	luapath::Table characterTable = settings.getGlobalTable("characterProfiles").getTable("." + profile);
	m_Direction = Direction::FORWARD;
	m_MinVelocity = characterTable.getValue(".minVelocity");
	m_MaxRunVelocity = characterTable.getValue(".maxRunVelocity");
	m_MaxWalkVelocity = characterTable.getValue(".maxWalkVelocity");
	m_AccelerationRun = characterTable.getValue(".accelerationRun");
	m_AccelerationWalk = characterTable.getValue(".accelerationWalk");
	m_AnimationSpeedModifier = characterTable.getValue(".accelerationWalk");
	m_Velocity = m_MinVelocity;

	//load attachments
	m_Primary.m_Parent = this;
	m_Primary.m_Play = false;
	m_Primary.m_TimeExpired = 0.0f;
	m_Primary.m_TotalWeaponOffset = 0.0f;
	m_Secondary.m_Parent = this;
	m_Secondary.m_TimeExpired = 0.0f;
	m_Secondary.m_Play = false;
	m_Secondary.m_TotalWeaponOffset = 0.0f;
	m_Primary.loadAttachment(characterTable.getTable(".primary"));
	m_Secondary.loadAttachment(characterTable.getTable(".secondary"));

	//load healhbar
	m_HealthBar.load(this);
}

Character::~Character()
{
	delete m_Primary.m_Object;
	delete m_Secondary.m_Object;
}



void Character::render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix)
{
	m_Primary.m_Object->render(viewMatrix, projMatrix);
	m_Secondary.m_Object->render(viewMatrix, projMatrix);
	m_Primary.m_Curve.render();
	m_HealthBar.render();
	//m_Secondary.m_Curve.render();
	SkinnedObject::render(viewMatrix, projMatrix);
}



void Character::update()
{
	SkinnedObject::update();
	m_Primary.updateAttachment();
	m_Secondary.updateAttachment();

}
bool Character::intersect(const AABB &other)
{
	if(m_State == State::ACTIVE)
		return m_AABB.intersect(other);
	return false;
}


void Character::playPrimary()
{
	if (m_Primary.m_TimeExpired <= 0.0f)
	{
		m_Primary.m_SavedWeaponsRotation = m_Primary.m_Object->getTransform().getOrientation();
		m_Primary.m_Play = true;
	}
}

void Attachment::loadAttachment(const luapath::Table &table)
{
	m_BoneAttachment = m_Parent->m_SkinnedModel->m_Skeleton->findBone(string(table.getValue(".boneAttach")));
	string Name = table.getValue(".modelName");
	m_Object = new Object(Name, Name);
	m_Object->generateAABB();
	m_OffsetPos.x = table.getValue(".position.x");
	m_OffsetPos.y = table.getValue(".position.y");
	m_OffsetPos.z = table.getValue(".position.z");
	m_OffsetRot.x = table.getValue(".rotation.x");
	m_OffsetRot.y = table.getValue(".rotation.y");
	m_OffsetRot.z = table.getValue(".rotation.z");

	//load IKs
	luapath::Table ikTable;
	if (table.getTable(".IK", ikTable))
	{
		string ikTypeStr = ikTable.getValue(".ikType");
		IKObject::IkType ikType;
		ikTypeStr == "global" ? ikType = IKObject::IkType::GLOBAL : ikType = IKObject::IkType::LOCAL;
		string boneEffector = ikTable.getValue(".boneEffector");
		int chainLength = ikTable.getValue(".chainLength");
		int maxTries = ikTable.getValue(".maxTries");
		m_DeltaWeaponOffset = ikTable.getValue(".deltaRotation");
		glm::vec3 position(ikTable.getValue(".position.x"), ikTable.getValue(".position.y"), ikTable.getValue(".position.z"));

		m_Speed = ikTable.getValue(".speed");

		glm::mat4x3 controlPoints;
		for (int i = 1; i <= 4; i++) 
		{
			luapath::Table controlPoint = ikTable.getTable(".controlPoints#" + std::to_string(i));
			controlPoints[i-1] = glm::vec3(controlPoint.getValue(".x"), controlPoint.getValue(".y"), controlPoint.getValue(".z"));
		}
		m_Curve.m_Type = CubicCurve::CurveType::Bezier;
		m_Curve.setControlPoints(controlPoints);
		m_Curve.setNumSamples((int)ikTable.getValue(".numSamples"));
		m_Parent->attachIK(ikType, boneEffector, position, chainLength, maxTries);
		
		m_BoneIk = m_Parent->m_SkinnedModel->m_Skeleton->findBone(boneEffector);
	}

}
void Attachment::updateAttachment()
{
	
	m_Curve.m_Transform = m_Parent->getTransform();
	SQTTransform boneTransform = m_Parent->getBoneGlobalTransform(m_BoneAttachment);
	glm::vec3 position = boneTransform.getPosition();

	if (m_Play)
	{
		if (m_TimeExpired >= 1.0f)
		{
			m_Play = false;
			m_TotalWeaponOffset = 0.0f;
//			m_Object->getTransform().setRotation(m_SavedWeaponsRotation); //restore rotation
			m_TimeExpired = 0.0f;
			m_Parent->getIK(m_BoneIk->m_Name).m_Position = m_Curve.getAtTime(0);
		}
		else
		{
			m_TotalWeaponOffset += m_DeltaWeaponOffset*Timer::get().getLastInterval();
			m_Object->getTransform().pivotOnLocalAxisDegrees(0,0,m_DeltaWeaponOffset); //add increment rotation
			m_Parent->getIK(m_BoneIk->m_Name).m_Position = m_Curve.getAtTime(m_TimeExpired); 
			m_TimeExpired += Timer::get().getLastInterval()*m_Speed;
		}
	}

	glm::quat rotation = boneTransform.getOrientation();
	m_Object->getTransform().setPosition(position);
	m_Object->getTransform().translateLocal(m_OffsetPos.x, m_OffsetPos.y, m_OffsetPos.z);
	m_Object->getTransform().setRotation(rotation);
	m_Object->getTransform().pivotOnLocalAxis(m_OffsetRot.x, m_OffsetRot.y, m_OffsetRot.z + m_TotalWeaponOffset);

	m_Object->m_AABB.transform(m_Object->getTransform().getMatrix());

}

HealthBar::HealthBar()
{
}
HealthBar::HealthBar(Character *parent, float yoffset, float width, float height, float breadth)
	:m_Parent(parent), m_Yoffset(yoffset), m_Width(width), m_Height(height), m_Breadth(breadth)
{

}

void HealthBar::load(Character *parent)
{
	m_Parent = parent;
	m_LastHitTime = 0;
	luapath::LuaState settings("config/settings.lua");
	luapath::Table healthBarTable = settings.getGlobalTable("healthBar");
	m_Width = healthBarTable.getValue(".width");
	m_Height = healthBarTable.getValue(".height");
	m_Breadth = healthBarTable.getValue(".breadth");
	m_Yoffset = healthBarTable.getValue(".yoffset");
	m_HealthLeft = healthBarTable.getValue(".healthLeft");
	m_HitTimeDelay = healthBarTable.getValue(".hitTimeDelay");
	string vertexShader = healthBarTable.getValue(".vertexShader");
	string fragmentShader = healthBarTable.getValue(".fragmentShader");
	m_Shader = ShaderManager::get().getShaderProgram(vertexShader, fragmentShader);

	//vertex spec
	float factor = m_HealthLeft;
	float halfWidth = m_Width/2;
	m_BoxMesh.m_Vertices.resize(8);
	m_BoxMesh.m_Vertices[0] = glm::vec3(-halfWidth, 0, 0);
	m_BoxMesh.m_Vertices[1] = glm::vec3(-halfWidth, 0, m_Breadth);
	m_BoxMesh.m_Vertices[2] = glm::vec3(-halfWidth, m_Height, m_Breadth);
	m_BoxMesh.m_Vertices[3] = glm::vec3(-halfWidth, m_Height, 0);

	m_BoxMesh.m_Vertices[4] = glm::vec3(halfWidth, 0, 0); 
	m_BoxMesh.m_Vertices[5] = glm::vec3(halfWidth, 0, m_Breadth); 
	m_BoxMesh.m_Vertices[6] = glm::vec3(halfWidth, 0, m_Breadth); 
	m_BoxMesh.m_Vertices[7] = glm::vec3(halfWidth, m_Height, 0); 

	//indices spec
	vector<GLuint> indices(36);
	GLuint indicesTemp[36] = 
	{
	0,1,2,
	0,2,3,
	0,3,7,
	0,4,7,
	0,4,5,
	0,1,5,
	4,5,6,
	4,6,7,
	2,3,6,
	3,6,7,
	1,2,6,
	1,5,6,
	};
	for (int i = 0; i < 36; i++)
	{
		indices[i] = indicesTemp[i];
	}
	m_BoxMesh.m_Indices = indices;

	m_GoodColor = glm::vec4(healthBarTable.getValue(".goodColor.r"),
		healthBarTable.getValue(".goodColor.g"),
		healthBarTable.getValue(".goodColor.b"),1.0f);
	m_BadColor = glm::vec4(healthBarTable.getValue(".badColor.r"),
		healthBarTable.getValue(".badColor.g"),
		healthBarTable.getValue(".badColor.b"),1.0f);
	
	m_BoxMesh.createVAO(m_Shader);
	m_BoxMesh.retrieveMaterialLocations(m_Shader);
}

void HealthBar::updateHealth(float factor)
{
	float currHitTime = Timer::get().getTime();
	if(currHitTime - m_LastHitTime > m_HitTimeDelay)
	{
		m_HealthLeft += factor;
		if(m_HealthLeft >= 1.0f) m_HealthLeft = 0.99f; 
		if(m_HealthLeft <= 0.0f)
		{
			m_HealthLeft = 0.0f;
			m_Parent->m_State = Object::State::DEACTIVE;
		}
		m_LastHitTime = currHitTime;
	}
}

void HealthBar::render()
{
	if (m_HealthLeft >= 0.0f)
	{
		SQTTransform goodBar = m_Parent->getTransform();
		goodBar.translateLocal(0, m_Yoffset, 0);
		goodBar.scaleUniform(m_HealthLeft);

		glUseProgram(m_Shader->m_Id);
		m_BoxMesh.m_Material.diffuse = m_GoodColor;
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(goodBar.getMatrix()));
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(GameWorld::get().getViewMatrix()));
		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(GameWorld::get().getProjectionMatrix()));
		m_BoxMesh.render(m_Shader, GL_TRIANGLE_STRIP, true);
	}

	SQTTransform badBar = m_Parent->getTransform();
	badBar.scaleUniform(1 - m_HealthLeft);
	glUseProgram(m_Shader->m_Id);
	m_BoxMesh.m_Material.diffuse = m_BadColor;
	//m_BoxMesh.m_Material.diffuse = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(badBar.getMatrix())); 
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(GameWorld::get().getViewMatrix())); 
	glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(GameWorld::get().getProjectionMatrix()));
	m_BoxMesh.render(m_Shader, GL_TRIANGLE_STRIP, true);

}