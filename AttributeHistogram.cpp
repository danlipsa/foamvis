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
    // does not work
    QwtScaleWidget* yLeftAxis = axisWidget (QwtPlot::yLeft);
    yLeftAxis->setBorderDist (100, 100);
    setFrameStyle (QFrame::Box | QFrame::Plain);
    setMargin (9);
    DisplayFocus (false);
}

void AttributeHistogram::DisplayFocus (bool focus)
{
    if (focus)
	setLineWidth (1);
    else
	setLineWidth (0);
    replot ();
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
    InteractionMode::Enum interactionMode = InteractionMode::Enum(index);
    if (interactionMode == InteractionMode::SELECT)
	SetSelectionTool (Histogram::BRUSH);
    else if (interactionMode == InteractionMode::DESELECT)
	SetSelectionTool (Histogram::ERASER);
    else
	SetSelectionTool (Histogram::NONE);
}

void AttributeHistogram::createActions ()
{
    m_actionSelectAll = boost::make_shared<QAction> (tr("&Select All"), this);
    m_actionSelectAll->setStatusTip(tr("Select All"));
    connect(m_actionSelectAll.get (), SIGNAL(triggered()),
	    this, SLOT(SelectAll ()));    

    m_actionDeselectAll = boost::make_shared<QAction> (
	tr("&Deselect All"), this);
    m_actionDeselectAll->setStatusTip(tr("Deselect All"));
    connect(m_actionDeselectAll.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectAll ()));

    m_actionHeightSettings = boost::make_shared<QAction> (
	tr("&Height Settings"), this);
    m_actionHeightSettings->setStatusTip(tr("Height Settings"));
    connect(m_actionHeightSettings.get (), SIGNAL(triggered()),
	    this, SLOT(HistogramSettingsDialog ()));
}

void AttributeHistogram::SetColorTransferFunction (
    boost::shared_ptr<ColorBarModel> colorBarModel)
{
    Histogram::SetColorTransferFunction (
	colorBarModel->GetInterval (), colorBarModel->GetColorMap ());
    replot ();
}
