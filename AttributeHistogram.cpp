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
#include "DebugStream.h"

AttributeHistogram::AttributeHistogram (QWidget* parent) :
    Histogram (parent)
{
    createActions ();
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
    m_actionSelectAll = boost::make_shared<QAction> (tr("&Select All"), this);
    m_actionSelectAll->setShortcut(
	QKeySequence (tr ("Shift+S")));
    m_actionSelectAll->setStatusTip(tr("Select All"));
    connect(m_actionSelectAll.get (), SIGNAL(triggered()),
	    this, SLOT(SelectAll ()));    

    m_actionDeselectAll = boost::make_shared<QAction> (
	tr("&Deselect All"), this);
    m_actionDeselectAll->setShortcut(
	QKeySequence (tr ("Shift+D")));
    m_actionDeselectAll->setStatusTip(tr("Deselect All"));
    connect(m_actionDeselectAll.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectAll ()));

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
