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
    void Step (ViewNumber::Enum viewNumber, int direction);
    void SetHistoryCount (size_t historyCount)
    {
	m_historyCount = historyCount;
    }
    size_t GetHistoryCount () const
    {
	return m_historyCount;
    }
    const GLWidget& GetGLWidget () const
    {
	return m_glWidget;
    }
    size_t GetCurrentHistoryCount () const
    {
	return m_currentHistoryCount;
    }

protected:
    virtual void init (ViewNumber::Enum viewNumber);
    virtual void addStep (ViewNumber::Enum viewNumber, size_t timeStep) = 0;
    virtual void removeStep (ViewNumber::Enum viewNumber, size_t timeStep) = 0;

private:
    const GLWidget& m_glWidget;
    size_t m_currentHistoryCount;
    size_t m_historyCount;
};

#endif //__AVERAGE_H__

// Local Variables:
// mode: c++
// End:
