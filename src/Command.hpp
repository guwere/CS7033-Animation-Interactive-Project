#pragma once
#include "stdafx.h"

class Object;
class Character;
/**An implementation of the Command pattern*/
class Command
{
public:
	virtual void execute() = 0;
protected:

};

// character commands
class CommandNoInput
	: public Command
{
public:
	CommandNoInput();
	virtual void execute();
private:

};


class CommandCharacterRun
	: public Command
{
public:
	CommandCharacterRun(std::string characterName, Direction direction);
	virtual void execute();
private:
	std::string m_Name;
	Direction m_Direction;
};

class CommandCharacterWalk
	: public Command
{
public:
	CommandCharacterWalk(std::string characterName, Direction direction);
	virtual void execute();
private:
	std::string m_Name;
	Direction m_Direction;
};

class CommandCharacterPrimary
	: public Command
{
public:
	CommandCharacterPrimary(std::string characterName);
	virtual void execute();
private:
	std::string m_Name;
};

class CommandCharacterSecondary
	: public Command
{
public:
	CommandCharacterSecondary(std::string characterName);
	virtual void execute();
private:
	std::string m_Name;
};

class CommandMouseMove
	: public Command
{
public:
	CommandMouseMove(double xPos, double yPos);
	virtual void execute();
private:
	double m_XPos, m_YPos;
};

class CommandPlayAnim
	: public Command
{
public:
	CommandPlayAnim(std::string characterName, std::string animationName);
	virtual void execute();
private:
	std::string m_Name;
	std::string m_AnimationName;
};

class CommandLevelCollision
	: public Command
{
public:
	CommandLevelCollision(std::string characterName);
	virtual void execute();
private:
	std::string m_Name;
};

class CommandObjectCollision
	: public Command
{
public:
	CommandObjectCollision(Object *object1, Object *object2);
	virtual void execute();
private:
	Object *m_Object1;
	Object *m_Object2;
};

class CommandWeaponCollision
	: public Command
{
public:
	CommandWeaponCollision(Character *object1, Character *object2);
	virtual void execute();
private:
	Character *m_Object1;
	Character *m_Object2;
};

//misc commands
