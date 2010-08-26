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
    menu.addAction (m_actionHeightSettings.get ());
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

void AttributeHistogram::HeightSettings ()
{
    m_attributeHistogramHeight->SetValue (GetMaxValueAxis ());
    m_attributeHistogramHeight->SetMaximumValue (GetMaxValueData ());
    m_attributeHistogramHeight->SetLogScale (IsLogValueAxis ());
    if (m_attributeHistogramHeight->exec () == QDialog::Accepted)
    {
	SetLogValueAxis (
	    m_attributeHistogramHeight->IsLogScale () ? true : false);
	SetMaxValueAxis (m_attributeHistogramHeight->GetValue ());
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
    m_actionHeightSettings = boost::make_shared<QAction> (
	tr("&Height Settings"), this);
    m_actionHeightSettings->setStatusTip(tr("Height Settings"));
    connect(m_actionHeightSettings.get (), SIGNAL(triggered()),
	    this, SLOT(HeightSettings ()));
}
