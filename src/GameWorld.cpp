#include "GameWorld.hpp"
#include "ModelManager.hpp"
#include "Player.hpp"
#include "Character.hpp"
#include "Camera.hpp"
//#include "Command.hpp"
#include "CommandQueue.hpp"
#include "Control.hpp"
#include "Timer.hpp"
#include "Curve.hpp"
#include "math_utilities.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <luapath\luapath.hpp>

#include <cstdlib>
#include <ctime>

using std::vector;
using std::string;

GameWorld& GameWorld::get()
{
	static GameWorld singleton;
	return singleton;
}

GameWorld::GameWorld()
	
{
	//random seed
	 srand (time(NULL));

	luapath::LuaState settings("config/settings.lua");
	m_PlayIntro = settings.getGlobalValue("playIntro");
	luapath::Value mode = settings.getGlobalValue("mode");
	if((string)mode == "DEBUG")
		setMode(DisplayMode::DEBUG);
	else
		setMode(DisplayMode::NORMAL);
	loadSkybox();
	loadLevel();
	loadEnemies();
	luapath::Table animationTable = settings.getGlobalTable("animation");
	m_BlendTime = animationTable.getValue(".blendTime");

	//m_Player = new Player();

	//..start of hack. Character initialization happens before the GameWorld has a chance of loading the debug info  and it includes curve initialization
	//and the CubicCurve class is badly designed as its initalization depends on the debug setting being loaded
	std::map<std::string, SkinnedObject*>::iterator it = m_SkinnedObjects.begin();
	for (; it != m_SkinnedObjects.end(); ++it)
	{
		Character *character = dynamic_cast<Character*>(it->second);
		if (character)
		{
			character->m_Primary.m_Curve.init();
		}
	
	}
	//end of hack
}

GameWorld::~GameWorld()
{
	std::map<std::string, Object*>::iterator it = m_AllObjects.begin();
	for (; it != m_AllObjects.end(); ++it)
		delete it->second;
	DebugObjectMap::iterator it3 = m_DebugObjects.begin();
	for (; it3 != m_DebugObjects.end(); ++it3)
		delete it3->second;
	delete m_Level;
	delete m_Gate;
	delete m_Skybox;
}

void GameWorld::updateWorldIntro()
{
	//static stuff
	static enum Intro{SETUP,PAN_CAMERA,GATE_OPEN,GATE_WAIT,MOVE1,MOVE1STOP,ROTATE1,MOVE2,MOVE2STOP,ROTATE2,GATE_CLOSE,START_ACTION};
	static float deltaTime = 0;
	static Intro currAction = SETUP;
	static float startEye = 50.0f;
	//static std::map<string,Character*> enemies;
	//PAN_CAMERA
	static float endEye = 3.0f;
	static float cameraPanTime = 5.0f;
	static float deltaDistanceCamera = -(abs(startEye - endEye) / cameraPanTime);
	//GATE_OPEN
	static float gateTop = 7.0f;
	static float gateBottom = -1.0f;
	static float gateOpenTime = 3.0f;
	static float deltaDistanceGateOpen = (abs(gateTop - gateBottom) / gateOpenTime);
	//GATE_WAIT
	static float waitTime = 1.0f;
	static float waitTimeExpired = 0.0f;
	//MOVE1
	static float move1Distance = 10.0f; // z
	static float move1Time = 3.0f;
	static float move1TimeExpired = 0;
	static float move1DeltaDistance = (move1Distance/ move1Time);
	//ROTATE1
	static float rotate1Rotation = 90.0f;
	static float rotate1Time = 2.0f;
	static float rotate1TotalRotation = 0;
	static float rotate1Delta = rotate1Rotation / rotate1Time;
	//MOVE2
	static float move2Distance = 5.0f; // z
	static float move2Time = 3.0f;
	static float move2TimeExpired = 0;
	static float move2DeltaDistance = -(move2Distance/ move2Time);
	//ROTATE2
	static float rotate2Rotation = 90.0f;
	static float rotate2Time = 2.0f;
	static float rotate2TotalRotation = 0;
	static float rotate2Delta = rotate2Rotation / rotate2Time;

	//GATE_CLOSE
	static float gateCloseTime = 3.0f;
	static float deltaDistanceGateClose = -(abs(gateTop - gateBottom) / gateCloseTime);
	
	std::map<std::string, Enemy*>::const_iterator enemy = m_Enemies.begin();

	Timer::get().updateInterval();
	deltaTime = Timer::get().getLastInterval();
	switch (currAction)
	{
	case SETUP:
		m_Player.getCharacter()->getTransform().pivotOnLocalAxisDegrees(0,180.0f,0);
		m_Player.m_EyeYOffset.y = startEye;

		m_Enemies["e1"]->getTransform().setPosition(-2.0f,-1.0f,-30.0f);
		m_Enemies["e2"]->getTransform().setPosition(0.0f,-1.0f,-30.0f);
		m_Enemies["e3"]->getTransform().setPosition(2.0f,-1.0f,-28.0f);
		m_Enemies["e4"]->getTransform().setPosition(-2.0f,-1.0f,-28.0f);
		m_Enemies["e5"]->getTransform().setPosition(0.0f,-1.0f,-28.0f);
		m_Enemies["e6"]->getTransform().setPosition(2.0f,-1.0f,-30.0f);
		currAction = PAN_CAMERA;
		break;
	case PAN_CAMERA:
		m_Player.m_EyeYOffset.y += deltaDistanceCamera * deltaTime;
		if(m_Player.m_EyeYOffset.y < endEye)
			currAction = GATE_OPEN;
		break;
	case GATE_OPEN:
		m_Gate->getTransform().translateLocal(0,deltaDistanceGateOpen * deltaTime,0);
		if(m_Gate->getTransform().getPosition().y > gateTop)
			currAction = GATE_WAIT; 
		break;
	case GATE_WAIT:
		waitTimeExpired += deltaTime;
		if(waitTimeExpired < waitTime)
			currAction = MOVE1;
		break;
	case MOVE1:
		{
		float currDelta = move1DeltaDistance*deltaTime;
		for(; enemy != m_Enemies.end(); ++enemy)
		{
			if(enemy->second->getCurrentAnim()->m_Name != "run")
				enemy->second->playAnimBlend("run",1.5f);
			enemy->second->getTransform().translateLocal(0,0,currDelta);
		}
		move1TimeExpired += deltaTime;
		if(move1TimeExpired > move1Time)
			currAction = MOVE1STOP;
		}
		break;
	case MOVE1STOP:
		for(; enemy != m_Enemies.end(); ++enemy)
			enemy->second->flushAnimQueue();
		currAction = ROTATE1;
		break;
	case ROTATE1:
		{
		float currRotation = rotate1Delta * deltaTime;
		m_Enemies["e1"]->getTransform().pivotOnLocalAxisDegrees(0,currRotation,0);
		m_Enemies["e2"]->getTransform().pivotOnLocalAxisDegrees(0,currRotation,0);
		m_Enemies["e3"]->getTransform().pivotOnLocalAxisDegrees(0,currRotation,0);
		m_Enemies["e4"]->getTransform().pivotOnLocalAxisDegrees(0,-currRotation,0);
		m_Enemies["e5"]->getTransform().pivotOnLocalAxisDegrees(0,-currRotation,0);
		m_Enemies["e6"]->getTransform().pivotOnLocalAxisDegrees(0,-currRotation,0);
		rotate1TotalRotation += currRotation;
		if(rotate1TotalRotation > rotate1Rotation)
			currAction = MOVE2;
		break;
		}
	case MOVE2:
		{
		float currDelta = move2DeltaDistance*deltaTime;
		for(; enemy != m_Enemies.end(); ++enemy)
		{
			if(enemy->second->getCurrentAnim()->m_Name != "run")
				enemy->second->playAnimBlend("run",1.5f);
			enemy->second->getTransform().translateLocal(0,0,-currDelta);
		}
		move2TimeExpired += deltaTime;
		if(move2TimeExpired > move2Time)
			currAction = MOVE2STOP;
		}
		break;
	case MOVE2STOP:
		for(; enemy != m_Enemies.end(); ++enemy)
			enemy->second->flushAnimQueue();
		currAction = ROTATE2;
		break;
	case ROTATE2:		
		{
		float currRotation = rotate2Delta * deltaTime;
		m_Enemies["e1"]->getTransform().pivotOnLocalAxisDegrees(0,-currRotation,0);
		m_Enemies["e2"]->getTransform().pivotOnLocalAxisDegrees(0,-currRotation,0);
		m_Enemies["e3"]->getTransform().pivotOnLocalAxisDegrees(0,-currRotation,0);
		m_Enemies["e4"]->getTransform().pivotOnLocalAxisDegrees(0,currRotation,0);
		m_Enemies["e5"]->getTransform().pivotOnLocalAxisDegrees(0,currRotation,0);
		m_Enemies["e6"]->getTransform().pivotOnLocalAxisDegrees(0,currRotation,0);
		rotate2TotalRotation += currRotation;
		if(rotate2TotalRotation > rotate2Rotation)
			currAction = GATE_CLOSE;
		}
		break;
	case GATE_CLOSE:
		m_Gate->getTransform().translateLocal(0,deltaDistanceGateClose * deltaTime,0);
		if(m_Gate->getTransform().getPosition().y < gateBottom)
			currAction = START_ACTION;
		break;
	case START_ACTION:
		m_PlayIntro = false;
		break;
	}
	//case independent updates
	
	m_Player.getCharacter()->update();
	for(enemy = m_Enemies.begin(); enemy != m_Enemies.end(); ++enemy)
	{
		enemy->second->update();
	}
	render();
	CommandQueue::get().process();
	setViewMatrix(m_Player.getViewMatrix());


}

void GameWorld::updateWorld()
{
	Timer::get().updateInterval();
    Control::get().handleInput();
	CommandQueue::get().process();
	setViewMatrix(m_Player.getViewMatrix());

	
	//collision detection
	//magic!
	std::map<string,string> collisionMap; // keeps track of which objects have collided
	std::map<std::string, Object*>::const_iterator obj = m_AllObjects.begin();
	
	for(;obj != m_AllObjects.end(); ++obj)
	{
		if (obj->second->m_State == Object::State::DEACTIVE)
			continue;
		obj->second->update(); // please don't forget: don't do updating in the render method
		if(glm::length(obj->second->getTransform().getPosition()) > m_LevelRadius)
		{
			CommandQueue::get().addCommandDisposable(new CommandLevelCollision(obj->second->m_Name));
		}

		std::map<std::string, Object*>::const_iterator obj2 = m_AllObjects.begin();
		for(;obj2 != m_AllObjects.end(); ++obj2)
		{
			if(obj != obj2) // the same object
			{
				if(obj->second->intersect(obj2->second->m_AABB))
				{
					if(collisionMap.find(obj->second->m_Name) == collisionMap.end())
					{
						bool alreadyCollided = false;
						std::map<string,string>::const_iterator collision = collisionMap.begin();
						for(;collision != collisionMap.end(); ++collision)
						{
							if(collision->second == obj->second->m_Name)
							{
								alreadyCollided = true;
								break;
							}
						}
						if(!alreadyCollided)
						{
							collisionMap[obj->second->m_Name] = obj2->second->m_Name;
							CommandQueue::get().addCommandDisposable(new CommandObjectCollision(obj->second, obj2->second));
						}
					}
				}
			}
		}

	}

	//weapon to character collision
	std::map<std::string, Enemy*>::const_iterator enemy = m_Enemies.begin();
	for(; enemy != m_Enemies.end(); ++enemy)
	{
		//glm::vec3 directionToPlayer = glm::length(enemy->second->getTransform().getPosition - m_Player.getCharacter()->getTransform().getPosition());
		//if(glm::length(directionToPlayer) < 3.0f)
		//{
		//	float yRot = glm::dot(glm::normalize(directionToPlayer), glm::normalize(enemy->second->getTransform().getForwardDirection()));
		//	enemy->second->getTransform().pivotOnLocalAxis(glm::angleAxis(acos(yRot),glm::vec3(0.0f,1.0f,0.0f)));
		//}
		Attachment &playerWeapon = m_Player.getCharacter()->m_Primary;
		if(playerWeapon.m_Play &&  playerWeapon.m_Object->m_AABB.intersect(enemy->second->m_AABB))
		{
			CommandQueue::get().addCommandDisposable(new CommandWeaponCollision(m_Player.getCharacter(), enemy->second));
		}
		
		Attachment &enemyWeapon = enemy->second->m_Primary;
		if(enemyWeapon.m_Play &&  enemyWeapon.m_Object->m_AABB.intersect(m_Player.getCharacter()->m_AABB))
		{
			CommandQueue::get().addCommandDisposable(new CommandWeaponCollision(enemy->second, m_Player.getCharacter()));
		}

	}



	render();

}


void GameWorld::render() const
{
	m_Level->render(m_ViewMatrix, m_ProjMatrix);
	m_Gate->render(m_ViewMatrix, m_ProjMatrix);
	m_Skybox->render(m_ViewMatrix, m_ProjMatrix);


	std::map<std::string, Object*>::const_iterator it = m_AllObjects.begin();
	for (; it != m_AllObjects.end(); ++it)
	{
		it->second->render(m_ViewMatrix, m_ProjMatrix);
	}

	if(m_DebugEnabled)
		renderDebug();

	
}

void GameWorld::renderDebug() const
{
	std::map<std::string, Object*>::const_iterator it = m_Objects.begin();
	for (; it != m_Objects.end(); ++it)
	{
		if(m_DebugTypeEnabled.at(DebugObject::localCoordAxis))
		{
			m_DebugObjects.at(DebugObject::localCoordAxis)->getTransform().setRotation(it->second->getTransform().getOrientation());
			m_DebugObjects.at(DebugObject::localCoordAxis)->getTransform().setPosition(it->second->getTransform().getPosition());
			m_DebugObjects.at(DebugObject::localCoordAxis)->render(m_ViewMatrix, m_ProjMatrix);
		}
	}

	if(m_DebugTypeEnabled.at(DebugObject::ikTarget))
	{
		//render debug objects
		//const Object* ikModel = m_DebugObjects.at("ikObject");
		std::map<std::string, SkinnedObject*>::const_iterator it2 = m_SkinnedObjects.begin();
		for (; it2 != m_SkinnedObjects.end(); ++it2)
		{
			const SkinnedObject::IKObjectMap& currIK = it2->second->getAllIKs();
			SkinnedObject::IKObjectMap::const_iterator ikIt = currIK.begin();
			for (; ikIt != currIK.end(); ++ikIt)
			{
				m_DebugObjects.at(DebugObject::ikTarget)->getTransform().setPosition(ikIt->second.getPosition());
				m_DebugObjects.at(DebugObject::ikTarget)->render(m_ViewMatrix, m_ProjMatrix);
			}

		}
	}

	if(m_DebugTypeEnabled.at(DebugObject::globalCoordAxis))
	{
		m_DebugObjects.at(DebugObject::globalCoordAxis)->render(m_ViewMatrix, m_ProjMatrix);
	}

	
}


Object* GameWorld::getObject(const std::string &objectName) const
{
	
	std::map<std::string, Object*>::const_iterator it = m_AllObjects.find(objectName);
	if(it != m_AllObjects.end())
		return it->second;
	return NULL;
}

SkinnedObject* GameWorld::getSkinnedObject(const std::string &objectName) const
{
	return m_SkinnedObjects.at(objectName);
}

float GameWorld::getBlendTime() const
{
	return m_BlendTime;
}


void GameWorld::addObject(Object *object)
{
	m_Objects[object->m_Name] = object;
	m_AllObjects[object->m_Name] = object;
}
void GameWorld::addSkinnedObject(SkinnedObject *object)
{
	m_SkinnedObjects[object->m_Name] = object;
	m_AllObjects[object->m_Name] = object;
}

void GameWorld::addEnemy(Enemy *object)
{
	m_Enemies[object->m_Name] = object;
	m_AllObjects[object->m_Name] = object;
}


void GameWorld::setMode(DisplayMode mode)
{
	if(mode == DisplayMode::DEBUG && !m_DebugEnabled)
	{
		
		m_DebugEnabled = loadDebugSettings();
		if(!m_DebugEnabled)
		{
			LOG(ERROR) << "Could not load debug settings. Debug Mode disabled";
			m_CurrentMode = DisplayMode::NORMAL;
		}
		else
		{
			m_CurrentMode = DisplayMode::DEBUG;
		}
	}
	else if (mode == DisplayMode::NORMAL)
	{
		m_CurrentMode = DisplayMode::NORMAL;
		m_DebugEnabled = false;
	}


}
void GameWorld::setViewMatrix(const glm::mat4 &view)
{
	m_ViewMatrix = view;
}
glm::mat4 GameWorld::getViewMatrix() const
{
	return m_ViewMatrix;
}

void GameWorld::setProjectionMatrix(const glm::mat4 &projection)
{
	m_ProjMatrix = projection;
}
glm::mat4 GameWorld::getProjectionMatrix() const
{
	return m_ProjMatrix;
}

Player& GameWorld::getPlayer()
{
	return m_Player;
}


/**@brief Retrieve object to be rendered for a particular type of debug entity like an object representing the ik position*/
Object* GameWorld::getDebugObject(GameWorld::DebugObject objectType) const
{
	return m_DebugObjects.at(objectType);
}
/**@brief Tells whether debugging for object of type @param objectType is enabled*/
bool GameWorld::isDebugTypeEnabled(GameWorld::DebugObject objectType) const
{
	return m_DebugTypeEnabled.at(objectType);
}
bool GameWorld::isDebugEnabled() const
{
	return m_DebugEnabled;
}


void GameWorld::loadSkybox()
{
	luapath::LuaState settings("config/settings.lua");
	luapath::Table skyboxTable = settings.getGlobalTable("skybox");

	string modelName = skyboxTable.getValue(".model");
	float scale = skyboxTable.getValue(".scale");

	Object* object = new Object("skybox",modelName,SQTTransform(glm::vec3(0),glm::vec3(scale),glm::quat()));
	m_Skybox = object;


}

void GameWorld::loadLevel()
{
	luapath::LuaState settings("config/settings.lua");
	luapath::Table levelTable = settings.getGlobalTable("level");
	m_LevelRadius = levelTable.getValue(".radius");
	string levelModelName = levelTable.getValue(".modelName");
	glm::vec3 position(levelTable.getValue(".position.x"),levelTable.getValue(".position.y"),levelTable.getValue(".position.z"));
	float scale = levelTable.getValue(".scale");
	m_Level = new Object("level",levelModelName,SQTTransform(position,glm::vec3(scale),glm::quat()));
	
	//...start of hack
	m_Gate = new Object("gate","gate",SQTTransform(position,glm::vec3(scale),glm::quat()));
	//end of hack
}

void GameWorld::loadIntro()
{

	//if intro events expired
	//play intro <- false
}

void GameWorld::loadEnemies()
{
	luapath::LuaState settings("config/settings.lua");
	luapath::Table enemyTable = settings.getGlobalTable("enemies");
	luapath::Table currEnemy;
	unsigned int currNum = 1;
 	while(enemyTable.getTable(".models#" + std::to_string(currNum),currEnemy))
	{
		string enemyName = currEnemy.getValue(".name");
		string modelName = currEnemy.getValue(".modelName");
		string characterProfile = currEnemy.getValue(".characterProfile");
		float x = randomNumber(-m_LevelRadius,m_LevelRadius);
		//float x = 0;
		float y = m_Level->getTransform().getPosition().y;
		float z = randomNumber(-m_LevelRadius,m_LevelRadius);
		//float z = 0;
		glm::vec3 position(x,y,z);
		glm::quat rotation = glm::angleAxis(randomNumber(-180.0f, 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Enemy *newEnemy = new Enemy(characterProfile, enemyName, modelName, SQTTransform(position,glm::vec3(1),glm::quat()));
		//Enemy *newEnemy = new Enemy(characterProfile, enemyName, modelName, SQTTransform());
		newEnemy->generateAABB();
		addEnemy(newEnemy);
		currNum++;
	}
}

bool GameWorld::loadDebugSettings()
{
	luapath::LuaState settings("config/settings.lua");
	luapath::Table debugTable = settings.getGlobalTable("debug");

	//get the models representing the axis. For now debug axis is treated as an object rather than constructing lines and whatnot on the fly
	loadDebugDisplaySetting(debugTable,"globalCoordAxis",DebugObject::globalCoordAxis,glm::vec3(-3));
	loadDebugDisplaySetting(debugTable,"localCoordAxis",DebugObject::localCoordAxis,glm::vec3(0));
	loadDebugDisplaySetting(debugTable,"boneCoordAxis",DebugObject::boneCoordAxis,glm::vec3(0));
	loadDebugDisplaySetting(debugTable,"curvePoint",DebugObject::curvePoint,glm::vec3(0));
	loadDebugDisplaySetting(debugTable,"ikTarget",DebugObject::ikTarget,glm::vec3(0));

	return true;
}

void GameWorld::loadDebugDisplaySetting(const luapath::Table &debugTable, const std::string &debugSettingName, GameWorld::DebugObject objectType, glm::vec3 position)
{
	//get the models representing the axis. For now debug axis is treated as an object rather than constructing lines and whatnot on the fly
	float scale = debugTable.getValue("." + debugSettingName + ".scale");
	string modelName = debugTable.getValue("." + debugSettingName + ".model");
	bool enable = debugTable.getValue("." + debugSettingName + ".enable");
	m_DebugTypeEnabled[objectType] = enable;
	Model* model = ModelManager::get().getModel(modelName);
	Object* object = new Object(debugSettingName,model,SQTTransform(glm::vec3(position),glm::vec3(scale),glm::quat()));
	m_DebugObjects[objectType] = object;
}

