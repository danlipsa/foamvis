/**
 * @file   AttributeHistogram.cpp
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Definition of the AttributeHistogram class
 */

#include "AttributeHistogram.h"
#include "Enums.h"

AttributeHistogram::AttributeHistogram (QWidget* parent) :
    Histogram (parent)
{
}

void AttributeHistogram::SetActionSelectAll (
    boost::shared_ptr<QAction> actionSelectAll)
{
    m_actionSelectAll = actionSelectAll;
    connect(m_actionSelectAll.get (), SIGNAL(triggered()),
	    this, SLOT(SelectAll ()));    
}

void AttributeHistogram::SetActionDeselectAll (
    boost::shared_ptr<QAction> actionDeselectAll)
{
    m_actionDeselectAll = actionDeselectAll;
    connect(m_actionDeselectAll.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectAll ()));
}

void AttributeHistogram::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu (this);
    menu.addAction (m_actionSelectAll.get ());
    menu.addAction (m_actionDeselectAll.get ());
    menu.exec (event->globalPos());
}

void AttributeHistogram::SelectAll ()
{
    SetSelected (true);
}

void AttributeHistogram::DeselectAll ()
{
    SetSelected (false);
}

void AttributeHistogram::CurrentIndexChangedInteractionMode (int index)
{
    InteractionMode::Enum interactionMode = 
	static_cast<InteractionMode::Enum>(index);
    if (interactionMode == InteractionMode::SELECT_BRUSH)
    {
	EnableSelection ();
	SetSelectionTool (BRUSH);
    }
    else if (interactionMode == InteractionMode::SELECT_ERASER)
    {
	EnableSelection ();
	SetSelectionTool (ERASER);
    }
    else
	EnableSelection (false);
}
