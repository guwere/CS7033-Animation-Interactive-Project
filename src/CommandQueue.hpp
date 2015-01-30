#pragma once
#include "Command.hpp"

#include <queue>

class Command;
class CommandQueue
{
public:
	static CommandQueue& get();
	void process();
	void addCommand(Command *command);
	void addCommandDisposable(Command *command);
	virtual ~CommandQueue();
private:
	CommandQueue();
private:
	typedef std::queue<Command*> ConstantQueue;
	ConstantQueue m_ConstantQueue;
	typedef std::queue<Command*> DisposableQueue;
	DisposableQueue m_DisposableQueue;

};