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
    const QwtDoubleInterval& interval = model->GetInterval ();
    QwtLinearScaleEngine scaleEngine;
    QwtScaleDiv scaleDiv;
    const int maxMajorTicks = 8;
    const int maxMinorTicks = 5;
    const int fontSize = 12;
/*
    scaleDiv = scaleEngine.divideScale (interval.minValue (), 
    interval.maxValue (), 0, 0);
    QwtValueList majorTicks;
    majorTicks += (interval.minValue () + interval.maxValue()) / 2;
    majorTicks += interval.minValue ();
    majorTicks += interval.maxValue ();
    scaleDiv.setTicks(QwtScaleDiv::MajorTick, majorTicks);    
*/

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
    menu.addAction (m_actionSettings.get ());
    menu.exec (event->globalPos());    
}

// Slots and slot like methods
// ======================================================================

void ColorBar::Settings ()
{
    m_editTransferFunction->show ();
}
