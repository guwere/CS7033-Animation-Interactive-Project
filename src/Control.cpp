#include "Control.hpp"
#include "CommandQueue.hpp"
#include "Player.hpp"
#include <GLFW\glfw3.h>

using std::set;
Control& Control::get()
{
	static Control singleton;
	return singleton;
}
Control::Control()
{

}

Control::~Control()
{
	//KeyMap::iterator it = m_KeyMap.begin();
	//for(; it != m_KeyMap.end(); ++it)
	//{
	//	delete it->second;
	//}

}


void Control::subscribeCommand(Command *command,Control::GLFWKey key)
{
	set<GLFWKey> keySet;
	keySet.insert(key);
	addCommand(m_KeyMap, keySet, command);

}

void Control::subscribeCommand(Command *command, Control::GLFWKey key1, Control::GLFWKey key2)
{
	set<GLFWKey> keySet;
	if(key1 != GLFW_KEY_LEFT_SHIFT) keySet.insert(key1);
	if(key2 != GLFW_KEY_LEFT_SHIFT) keySet.insert(key2);
	if(key1 == GLFW_KEY_LEFT_SHIFT || key2 == GLFW_KEY_LEFT_SHIFT)
		addCommand(m_KeyShiftMap, keySet, command);
	else
		addCommand(m_KeyMap, keySet, command);
}

void Control::subscribeCommand(Command *command, Control::GLFWKey key1, Control::GLFWKey key2, Control::GLFWKey key3)
{
	set<GLFWKey> keySet;
	if(key1 != GLFW_KEY_LEFT_SHIFT) keySet.insert(key1);
	if(key2 != GLFW_KEY_LEFT_SHIFT) keySet.insert(key2);
	if(key3 != GLFW_KEY_LEFT_SHIFT) keySet.insert(key3);
	if(key1 == GLFW_KEY_LEFT_SHIFT || key2 == GLFW_KEY_LEFT_SHIFT || key3 == GLFW_KEY_LEFT_SHIFT)
		addCommand(m_KeyShiftMap, keySet, command);
	else
		addCommand(m_KeyMap, keySet, command);
}

void Control::addCommand(Control::KeyMap &map, std::set<GLFWKey> keys, Command *command)
{
	KeyMap::iterator it = map.find(keys);
	if(it != map.end())
		delete it->second;
	map[keys] = command;
}


void Control::press(Control::GLFWKey key)
{
	m_Keys[key] = true;
}
void Control::release(Control::GLFWKey key)
{
	m_Keys[key] = false;
}

void Control::mousePosition(double xPos, double yPos)
{
	m_MouseXPos = xPos;
	m_MmouseYPos = yPos;
}

void Control::handleInput()
{
	unsigned int keyPressed = 0;
	CommandQueue::get().addCommandDisposable(new CommandMouseMove(m_MouseXPos,m_MmouseYPos));
	if(m_Keys[GLFW_KEY_LEFT_SHIFT])
	{
		KeyMap::const_iterator commandIt = m_KeyShiftMap.begin();
		//iterate over the existing commands and see which have all their buttons pressed
		for(; commandIt != m_KeyShiftMap.end(); ++commandIt)
		{
			std::set<GLFWKey>::const_iterator keySetIt = commandIt->first.begin();
			bool allPressed = true;
			for(; keySetIt != commandIt->first.end(); ++keySetIt)
			{
				if(!m_Keys[*keySetIt]) // if not pressed
				{
					allPressed = false;
					break;
				}
			}
			if(allPressed)
			{
				CommandQueue::get().addCommand(commandIt->second);
				keyPressed++;
			}
		}
	}
	else // shift not pressed
	{
		KeyMap::const_iterator commandIt = m_KeyMap.begin();
		//iterate over the existing commands and see which have all their buttons pressed
		for(; commandIt != m_KeyMap.end(); ++commandIt)
		{
			std::set<GLFWKey>::const_iterator keySetIt = commandIt->first.begin();
			bool allPressed = true;
			for(; keySetIt != commandIt->first.end(); ++keySetIt)
			{
				if(!m_Keys[*keySetIt]) // if not pressed
				{
					allPressed = false;
					break;
				}
			}
			if(allPressed)
			{
				CommandQueue::get().addCommand(commandIt->second);
				keyPressed++;
			}
		}
	}

	if(!keyPressed)
		CommandQueue::get().addCommandDisposable(new CommandNoInput());

}

void Control::dispatchCommands()
{

}
