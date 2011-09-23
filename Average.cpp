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
#include "OpenGLUtils.h"

void Average::AverageInit (ViewNumber::Enum viewNumber)
{
    (void)viewNumber;
    m_currentTimeWindow = 0;
}



void Average::loopOperation (Operation op, 
			     ViewNumber::Enum viewNumber, size_t currentTime)
{
    size_t stepSize = getStepSize (currentTime);
    for (size_t i = 0; i < stepSize; ++i)
	(this->*op) (viewNumber, currentTime, i);
}

void Average::AverageStep (ViewNumber::Enum viewNumber, int timeDifference)
{
    if (abs (timeDifference) > 1)
    {
	AverageInitStep (viewNumber);
	return;
    }
    Operation first, second;
    size_t currentTime = m_glWidget.GetCurrentTime ();
    if (timeDifference < 0)
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
    loopOperation (first, viewNumber, currentTime);
    if (m_currentTimeWindow >= m_timeWindow && currentTime >= m_timeWindow)
    {
	currentTime -= m_timeWindow;
	loopOperation (second, viewNumber, currentTime);
    }
    else
	m_currentTimeWindow += timeDifference;
    WarnOnOpenGLError ("AverageStep");
}

