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
    m_currentHistoryCount = 0;
}

void Average::InitStep (ViewNumber::Enum viewNumber)
{
    init (viewNumber);
    Step (viewNumber, 1);
}

typedef void (Average::*Operation) (ViewNumber::Enum viewNumber, 
				    size_t timeStep);

void Average::Step (ViewNumber::Enum viewNumber, int direction)
{
    if (abs (direction) > 1)
    {
	InitStep (viewNumber);
	return;
    }
    Operation first, second;
    size_t timeStep = m_glWidget.GetTimeStep ();
    if (direction < 0)
    {
	++timeStep;
	first = &Average::removeStep;
	second = &Average::addStep;
    }
    else
    {
	first = &Average::addStep;
	second = &Average::removeStep;
    }
    (this->*first) (viewNumber, timeStep);
    if (m_currentHistoryCount >= m_historyCount && timeStep >= m_historyCount)
    {
	(this->*second) (viewNumber, timeStep - m_historyCount);
    }
    else
	m_currentHistoryCount += direction;
}
