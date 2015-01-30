#include "Timer.hpp"

#include <GLFW/glfw3.h>

Timer& Timer::get()
{
	static Timer singleton;
	return singleton;
}

Timer::Timer()
	:m_LastInterval(0), m_LastTime(glfwGetTime())
{

}

double Timer::getTime() const
{
	return m_LastTime; 
}

double Timer::getLastInterval() const
{
	return m_LastInterval;
}
void Timer::updateInterval()
{
	double currTime = glfwGetTime();
	m_LastInterval = currTime - m_LastTime;
	m_LastTime = currTime;
}

void Timer::reset()
{
	m_LastInterval = 0;
	m_LastTime = glfwGetTime();
}

