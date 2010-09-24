/**
 * @file   AttributeHistogram.cpp
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Definition of the AttributeHistogram class
 */

#include "AttributeHistogram.h"
#include "ColorBarModel.h"
#include "Enums.h"

AttributeHistogram::AttributeHistogram (QWidget* parent) :
    Histogram (parent)
{
    createActions ();
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
    menu.addAction (m_actionHeightSettings.get ());
    menu.exec (event->globalPos());
}

void AttributeHistogram::SelectAll ()
{
    SetAllItemsSelection (true);
}

void AttributeHistogram::DeselectAll ()
{
    SetAllItemsSelection (false);
}

void AttributeHistogram::CurrentIndexChangedInteractionMode (int index)
{
    InteractionMode::Enum interactionMode = 
	static_cast<InteractionMode::Enum>(index);
    if (interactionMode == InteractionMode::SELECT_BRUSH)
	SetSelectionTool (BRUSH);
    else if (interactionMode == InteractionMode::SELECT_ERASER)
	SetSelectionTool (ERASER);
    else
	SetSelectionTool (NONE);
}

void AttributeHistogram::createActions ()
{
    m_actionHeightSettings = boost::make_shared<QAction> (
	tr("&Height Settings"), this);
    m_actionHeightSettings->setStatusTip(tr("Height Settings"));
    connect(m_actionHeightSettings.get (), SIGNAL(triggered()),
	    this, SLOT(HistogramHeightDialog ()));
}

void AttributeHistogram::ColorBarModelChanged (
    boost::shared_ptr<ColorBarModel> colorBarModel)
{
    SetColorMap (colorBarModel->GetInterval (), colorBarModel->GetColorMap ());
    replot ();
}
