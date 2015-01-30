#pragma once
#include <set>
#include <vector>
#include <map>
class Command;
/**
@brief A singleton representing the keyboard and mouse controls
*/
class Control
{
public:
	static Control& get();
	typedef int GLFWKey;
	typedef std::map<std::set<GLFWKey>, Command*> KeyMap;
	static const unsigned int MAX_KEYS = 400;
	virtual ~Control();
	//enum class PressType{REPEAT,SINGLE};
	void subscribeCommand(Command *command,Control::GLFWKey key);
	void subscribeCommand(Command *command, GLFWKey key1,GLFWKey key2);
	void subscribeCommand(Command *command, GLFWKey key1,GLFWKey key2, GLFWKey key3);
	void press(GLFWKey key);
	void release(GLFWKey key);
	void mousePosition(double xPos, double yPos);
	void handleInput();
private:
	Control();
	void dispatchCommands();
	void addCommand(Control::KeyMap &map, std::set<GLFWKey> keys, Command *command);
private:
	bool m_Keys[MAX_KEYS];
	double m_MouseXPos, m_MmouseYPos;
	KeyMap m_KeyMap;
	KeyMap m_KeyShiftMap;
	////these are commands which are suppose to happen on each event cycle
	//typedef std::vector<Command*> RepeatedCommands;
	//RepeatedCommands m_RepeatedCommands;
};
//	Control.subscribe(vector<SHIFT,W>,REPEAT/SINGLE,commandMoveCharacterForward("Player"));
