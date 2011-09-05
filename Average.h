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
    void AverageStep (ViewNumber::Enum viewNumber, int timeStep);
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
    virtual void addStep (ViewNumber::Enum viewNumber, size_t timeStep) = 0;
    virtual void removeStep (ViewNumber::Enum viewNumber, size_t timeStep) = 0;

private:
    const GLWidget& m_glWidget;
    size_t m_currentTimeWindow;
    size_t m_timeWindow;
};

#endif //__AVERAGE_H__

// Local Variables:
// mode: c++
// End:
