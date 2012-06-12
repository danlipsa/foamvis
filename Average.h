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
    Average (const GLWidget& widgetGl) :
    m_widgetGl (widgetGl)
    {
    }
    void AverageSetTimeWindow (size_t timeWindow)
    {
	m_timeWindow = timeWindow;
    }
    void AverageStep (ViewNumber::Enum viewNumber, int timeDifference);
    size_t GetTimeWindow () const
    {
	return m_timeWindow;
    }
    const GLWidget& GetGLWidget () const
    {
	return m_widgetGl;
    }
    size_t GetCurrentTimeWindow () const
    {
	return m_currentTimeWindow;
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber);

protected:
    //@todo write and add/remove in one operations instead of two.
    virtual void addStep (
	ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep) = 0;
    virtual void removeStep (ViewNumber::Enum viewNumber, 
			     size_t timeStep, size_t subStep) = 0;
    // a step is divident in stepSize subSteps
    virtual size_t getStepSize (
	ViewNumber::Enum viewNumber, size_t timeStep) const
    {
	(void)timeStep;(void)viewNumber;
	return 1;
    }

private:
    typedef void (Average::*Operation) (ViewNumber::Enum viewNumber, 
					size_t timeStep, size_t subStep);
    void loopOperation (Operation op, ViewNumber::Enum viewNumber, 
			size_t currentTime);



private:
    const GLWidget& m_widgetGl;
    size_t m_currentTimeWindow;
    size_t m_timeWindow;
};

#endif //__AVERAGE_H__

// Local Variables:
// mode: c++
// End:
