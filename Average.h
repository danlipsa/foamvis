/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Interface for the Average class
 */

#ifndef __AVERAGE_H__
#define __AVERAGE_H__

#include "Enums.h"
#include "AverageInterface.h"

class GLWidget;

class Average : public AverageInterface
{
public:
    Average (const GLWidget& glWidget) :
    m_glWidget (glWidget)
    {
    }
    void AverageSetTimeWindow (size_t historyCount)
    {
	m_timeWindow = historyCount;
    }
    void AverageStep (ViewNumber::Enum viewNumber, int timeDifference);
    size_t GetTimeWindow () const
    {
	return m_timeWindow;
    }
    const GLWidget& GetGLWidget () const
    {
	return m_glWidget;
    }
    size_t GetCurrentTimeWindow () const
    {
	return m_currentTimeWindow;
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber);

protected:
    virtual void addStep (
	ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep) = 0;
    virtual void removeStep (ViewNumber::Enum viewNumber, 
			     size_t timeStep, size_t subStep) = 0;
    // a step is divident in stepSize subSteps
    virtual size_t getStepSize (size_t timeStep) const
    {
	(void)timeStep;
	return 1;
    }

private:
    typedef void (Average::*Operation) (ViewNumber::Enum viewNumber, 
					size_t timeStep, size_t subStep);
    void loopOperation (Operation op, ViewNumber::Enum viewNumber, 
			size_t currentTime);



private:
    const GLWidget& m_glWidget;
    size_t m_currentTimeWindow;
    size_t m_timeWindow;
};

#endif //__AVERAGE_H__

// Local Variables:
// mode: c++
// End:
