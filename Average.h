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
class GLWidget;

class Average
{
public:
    Average (const GLWidget& glWidget) :
    m_glWidget (glWidget)
    {
    }
    
    void InitStep (ViewNumber::Enum viewNumber);
    void Step (ViewNumber::Enum viewNumber, int timeStep);
    void SetTimeWindow (size_t historyCount)
    {
	m_timeWindow = historyCount;
    }
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

protected:
    virtual void init (ViewNumber::Enum viewNumber);
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
