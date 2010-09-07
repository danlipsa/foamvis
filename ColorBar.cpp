/**
 * @file   ColorBar.cpp
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Definition of the ColorBar class
 */

#include "ColorBar.h"
#include "ColorBarModel.h"

ColorBar::ColorBar (QWidget* parent) : 
    QwtScaleWidget (parent)
{
    setColorBarEnabled (true);
    createActions ();
}

void ColorBar::createActions ()
{
    m_actionEditTransferFunction = boost::make_shared<QAction> (
	tr("&Edit Transfer Function"), this);
    m_actionEditTransferFunction->setStatusTip(tr("Edit Transfer Function"));
    connect(m_actionEditTransferFunction.get (), SIGNAL(triggered()),
	    this, SLOT(ShowEditTransferFunction ()));
}

void ColorBar::SetModel (boost::shared_ptr<ColorBarModel> model)
{
    m_model = model;
    QwtDoubleInterval interval = model->GetInterval ();
    QwtLinearScaleEngine scaleEngine;
    QwtScaleDiv scaleDiv;
    const int maxMajorTicks = 8;
    const int maxMinorTicks = 5;
    const int fontSize = 12;

    scaleDiv = scaleEngine.divideScale (
	interval.minValue (), interval.maxValue (), 
	maxMajorTicks, maxMinorTicks);
    setScaleDiv (scaleEngine.transformation (), scaleDiv);    
    setColorMap (interval, model->GetColorMap ());
    QwtText title (model->GetTitle ());
    QFont fttl(fontInfo().family(), fontSize, QFont::Bold);
    title.setFont (fttl);
    setTitle (title);
}


void ColorBar::contextMenuEvent (QContextMenuEvent *event)
{
    QMenu menu (this);
    menu.addAction (m_actionEditTransferFunction.get ());
    menu.exec (event->globalPos());    
}

// Slots and slot like methods
// ======================================================================

void ColorBar::ShowEditTransferFunction ()
{
    Q_EMIT EditTransferFunction ();
}
