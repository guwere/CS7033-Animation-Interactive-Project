#pragma once
#include "stdafx.h"
#include "GameObject.hpp"
#include "Player.hpp"
#include "Enemy.hpp"

#include <glm/glm.hpp>



/**
@brief Manages the lifetime of Object instances and acts a central point for managements. e.g enabling debug mode
@details The View and Projection matrices are set on this object. Implemented a singleton. Will see how this affects program design in the future. Arguably this class should not be a singleton so modularity is better preserved.
@todo this would be the center piece with controls the subsystems in the future
*/
class GameWorld
{
public:
	/**@brief do we want to display the helper things like dots for the IKs, mesh normals, etc.
		@details NORMAL means no auxiliary helper things are displayed. DEBUG means they are
	 */
	enum class DisplayMode{NORMAL, DEBUG};
	static GameWorld& get();

	/**@brief Update things that are under world's controls*/
	void updateWorld();

	/**@brief Before the action begins this is what is called */
	void updateWorldIntro();

	/**@brief Render all object in the world*/
	void render() const;

	Object* getObject(const std::string &objectName) const;
	SkinnedObject* getSkinnedObject(const std::string &objectName) const;
	/**Add an object to be rendered thereafter*/
	void addObject(Object *object);
	void addSkinnedObject(SkinnedObject *object);
	void addEnemy(Enemy *object);

	void setMode(DisplayMode mode);
	void setViewMatrix(const glm::mat4 &view);
	glm::mat4 getViewMatrix() const;
	void setProjectionMatrix(const glm::mat4 &projection);
	glm::mat4 getProjectionMatrix() const;
	Player& getPlayer();
	float getBlendTime() const;

	enum class DebugObject{ikTarget, curvePoint, boneCoordAxis, localCoordAxis, globalCoordAxis};
	/**@brief Retrieve object to be rendered for a particular type of debug entity like an object representing the ik position*/
	Object* getDebugObject(DebugObject objectType) const;
	/**@brief Tells whether debugging for object of type @param objectType is enabled*/
	bool isDebugTypeEnabled(DebugObject objectType) const;
	/**@brief Tells whether debugging is enabled*/
	bool isDebugEnabled() const;
public:
	Object *m_Level;
	Object *m_Gate;
	Object *m_Skybox;
	float m_LevelRadius;

	bool m_PlayIntro;
private:
	/** Initialize an empty world */
	GameWorld();
	~GameWorld();

	std::map<std::string, Object*> m_AllObjects;
	std::map<std::string, Object*> m_Objects;
	std::map<std::string, SkinnedObject*> m_SkinnedObjects;
	std::map<std::string, Enemy*> m_Enemies;
	std::map<std::string, Object*> m_Deactive;

	Player m_Player;

	//glm::vec3 m_ViewPosition;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ProjMatrix;

	DisplayMode m_CurrentMode;
	bool m_DebugEnabled;
	//object templates for debug objects
	typedef std::map<DebugObject, Object*> DebugObjectMap;
	DebugObjectMap m_DebugObjects; //!<object templates for debug objects
	typedef std::map<DebugObject, bool> DebugTypeEnabledMap;
	DebugTypeEnabledMap m_DebugTypeEnabled; //!<object templates for debug objects
	float m_BlendTime;

	//intro sequences members

private:
	void loadIntro();
	void loadSkybox();
	void loadLevel();
	void loadEnemies();
	void loadDebugDisplaySetting(const luapath::Table &debugTable, const std::string &debugSettingName, GameWorld::DebugObject objectType, glm::vec3 position);
	void renderDebug() const;
	bool loadDebugSettings();
};