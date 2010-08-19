/**
 * @file   AttributeHistogram.cpp
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Definition of the AttributeHistogram class
 */

#include "AttributeHistogram.h"
#include "AttributeHistogramHeight.h"
#include "Enums.h"

AttributeHistogram::AttributeHistogram (QWidget* parent) :
    Histogram (parent)
{
    createActions ();
    m_attributeHistogramHeight = 
	boost::make_shared<AttributeHistogramHeight> (this);
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
    menu.addAction (m_actionAdjustHeight.get ());
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

void AttributeHistogram::AdjustHeight ()
{
    if (m_attributeHistogramHeight->exec () == QDialog::Accepted)
    {
	
    }
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

void AttributeHistogram::createActions ()
{
    m_actionAdjustHeight = boost::make_shared<QAction> (
	tr("&Adjust Height"), this);
    m_actionAdjustHeight->setStatusTip(tr("Adjust Height"));
    connect(m_actionAdjustHeight.get (), SIGNAL(triggered()),
	    this, SLOT(AdjustHeight ()));
}
