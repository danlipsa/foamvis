/**
 * @file   ColorBar.cpp
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Definition of the ColorBar class
 */

#include "ColorBar.h"
#include "ColorBarModel.h"
#include "EditTransferFunction.h"

ColorBar::ColorBar (QWidget* parent) : 
    QwtScaleWidget (parent)
{
    setColorBarEnabled (true);
    createActions ();
    m_editTransferFunction = boost::make_shared<EditTransferFunction> (parent);
}

void ColorBar::createActions ()
{
    m_actionSettings = boost::make_shared<QAction> (
	tr("&Settings"), this);
    m_actionSettings->setStatusTip(tr("Settings"));
    connect(m_actionSettings.get (), SIGNAL(triggered()),
	    this, SLOT(Settings ()));
}

void ColorBar::SetModel (boost::shared_ptr<ColorBarModel> model)
{
    m_model = model;
    QwtLinearScaleEngine scaleEngine;
    const QwtDoubleInterval& interval = model->GetInterval ();
    QwtScaleDiv scaleDiv = scaleEngine.divideScale (
	interval.minValue (), interval.maxValue (), 0, 0);
    QwtValueList majorTicks;
    majorTicks += (interval.minValue () + interval.maxValue()) / 2;
    majorTicks += interval.minValue ();
    majorTicks += interval.maxValue ();
    scaleDiv.setTicks(QwtScaleDiv::MajorTick, majorTicks);    
    setScaleDiv (scaleEngine.transformation (), scaleDiv);    
    setColorMap (interval, model->GetColorMap ());
}


void ColorBar::contextMenuEvent (QContextMenuEvent *event)
{
    QMenu menu (this);
    menu.addAction (m_actionSettings.get ());
    menu.exec (event->globalPos());    
}

// Slots and slot like methods
// ======================================================================

void ColorBar::Settings ()
{
    
}
