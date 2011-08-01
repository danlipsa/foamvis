/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Implementation for the Average class 
 *
 */

#include "Average.h"
#include "GLWidget.h"

void Average::init (ViewNumber::Enum viewNumber)
{
    (void)viewNumber;
    m_currentTimeWindow = 0;
}


typedef void (Average::*Operation) (ViewNumber::Enum viewNumber, 
				    size_t timeStep);

void Average::InitStep (ViewNumber::Enum viewNumber)
{
    init (viewNumber);
    Step (viewNumber, 1);
}

void Average::Step (ViewNumber::Enum viewNumber, int timeStep)
{
    if (abs (timeStep) > 1)
    {
	InitStep (viewNumber);
	return;
    }
    Operation first, second;
    size_t currentTime = m_glWidget.GetTime ();
    if (timeStep < 0)
    {
	++currentTime;
	first = &Average::removeStep;
	second = &Average::addStep;
    }
    else
    {
	first = &Average::addStep;
	second = &Average::removeStep;
    }
    (this->*first) (viewNumber, currentTime);
    if (m_currentTimeWindow >= m_timeWindow && currentTime >= m_timeWindow)
    {
	(this->*second) (viewNumber, currentTime - m_timeWindow);
    }
    else
	m_currentTimeWindow += timeStep;
}
