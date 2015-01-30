#include "CommandQueue.hpp"
#include "Command.hpp"

CommandQueue& CommandQueue::get()
{
	static CommandQueue singleton;
	return singleton;
}

CommandQueue::CommandQueue()
{

}
CommandQueue::~CommandQueue()
{
	while(m_DisposableQueue.size() > 0)
	{
		Command *command = m_DisposableQueue.front();
		m_DisposableQueue.pop();
		delete command;
	}

}

void CommandQueue::process()
{
	//no reason why disposable has priority. just is
	while(m_DisposableQueue.size() > 0)
	{
		Command *command = m_DisposableQueue.front();
		m_DisposableQueue.pop();
		command->execute();
		delete command;
	}
	while(m_ConstantQueue.size() > 0)
	{
		Command *constCommand = m_ConstantQueue.front();
		m_ConstantQueue.pop();
		constCommand->execute();
	}
}
void CommandQueue::addCommand(Command *command)
{
	m_ConstantQueue.push(command);
}
void CommandQueue::addCommandDisposable(Command *command)
{
	m_DisposableQueue.push(command);
}
