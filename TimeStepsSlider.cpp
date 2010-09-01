/**
 * @file TimeStepsSlider.h
 * @author Dan R. Lipsa
 * @date 1 Sept. 2010
 *
 * Declaration of TimeStepSlider object
 */

#include "TimeStepsSlider.h"

TimeStepsSlider::TimeStepsSlider (QWidget* parent) :
    RestrictedRangeSlider (parent)
{
    SetTitle ("Time Steps");
    createActions ();
}

void TimeStepsSlider::contextMenuEvent (QContextMenuEvent *event)
{
    QMenu menu (this);
    menu.addAction (m_actionNextSelectedTimeStep.get ());
    menu.addAction (m_actionPreviousSelectedTimeStep.get ());
    menu.exec (event->globalPos());    
}

void TimeStepsSlider::createActions ()
{
    m_actionNextSelectedTimeStep = boost::make_shared<QAction> (
	tr("&Next selected time step"), this);
    m_actionNextSelectedTimeStep->setShortcut(
	QKeySequence (tr ("Ctrl+M")));
    m_actionNextSelectedTimeStep->setStatusTip(tr("&Next selected time step"));
    connect(m_actionNextSelectedTimeStep.get (), SIGNAL(triggered()),
	    this, SLOT(NextSelectedTimeStep ()));
    
    m_actionPreviousSelectedTimeStep = boost::make_shared<QAction> (
	tr("&Previous selected time step"), this);
    m_actionPreviousSelectedTimeStep->setShortcut(
	QKeySequence (tr ("Ctrl+N")));
    m_actionPreviousSelectedTimeStep->setStatusTip(
	tr("&Previous selected time step"));
    connect(m_actionPreviousSelectedTimeStep.get (), SIGNAL(triggered()),
	    this, SLOT(PreviousSelectedTimeStep ()));
}

void TimeStepsSlider::NextSelectedTimeStep ()
{
    NextSelected ();
}

void TimeStepsSlider::PreviousSelectedTimeStep ()
{
    PreviousSelected ();
}

