/**
 * @file   ColorBar.cpp
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Definition of the ColorBar class
 */

#include "ColorBar.h"
#include "ColorBarModel.h"
#include "DebugStream.h"

ColorBar::ColorBar (QWidget* parent) : 
    QwtScaleWidget (parent),
    m_model (new ColorBarModel())
{
    setColorBarEnabled (true);
    createActions ();
}

void ColorBar::createActions ()
{
    m_actionEditColorMap.reset (
	new QAction (tr("&Edit Color Map"), this));
    m_actionEditColorMap->setStatusTip(tr("Edit Color Map"));
    connect(m_actionEditColorMap.get (), SIGNAL(triggered()),
	    this, SLOT(ShowEditColorMap ()));

    m_actionClampClear.reset (
	new QAction (tr("&Clamp Clear"), this));
    m_actionClampClear->setStatusTip(tr("Clamp Clear"));
    connect(m_actionClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(ClampClear ()));
}

void ColorBar::ColorBarModelChangedMainWindow (
    boost::shared_ptr<ColorBarModel> model)
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
    menu.addAction (m_actionEditColorMap.get ());
    menu.addAction (m_actionClampClear.get ());
    menu.exec (event->globalPos());    
}

// Slots and slot like methods
// ======================================================================

void ColorBar::ShowEditColorMap ()
{
    Q_EMIT EditColorMap ();
}

void ColorBar::ClampClear ()
{
    m_model->SetClampClear ();
    m_model->SetupPalette (m_model->GetPalette ());
    setColorMap (m_model->GetInterval (), m_model->GetColorMap ());
    Q_EMIT ColorBarModelChanged (m_model);
}
