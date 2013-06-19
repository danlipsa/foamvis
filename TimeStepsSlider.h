/**
 * @file TimeStepsSlider.h
 * @author Dan R. Lipsa
 * @date 1 Sept. 2010
 * @brief A time step slider that allows only certain time steps to be enabled
 * @ingroup ui
 */
#ifndef __TIME_STEP_SLIDER_H__
#define __TIME_STEP_SLIDER_H__

#include "RestrictedRangeSlider.h"

/**
 * @brief A time step slider that allows only certain time steps to be enabled
 */
class TimeStepsSlider : public RestrictedRangeSlider
{
public:
    TimeStepsSlider (QWidget* parent);
    boost::shared_ptr<QAction> GetActionNextSelectedTimeStep () const
    {
	return m_actionNextSelectedTimeStep;
    }

    boost::shared_ptr<QAction> GetActionPreviousSelectedTimeStep () const
    {
	return m_actionPreviousSelectedTimeStep;
    }


public Q_SLOTS:
    void NextSelectedTimeStep ();
    void PreviousSelectedTimeStep ();

protected:
    virtual void contextMenuEvent (QContextMenuEvent *event);

private:
    void createActions ();

private:
    Q_OBJECT

    boost::shared_ptr<QAction> m_actionNextSelectedTimeStep;
    boost::shared_ptr<QAction> m_actionPreviousSelectedTimeStep;
};


#endif //__TIME_STEP_SLIDER_H__

// Local Variables:
// mode: c++
// End:
