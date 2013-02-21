/**
 * @file   AttributeHistogram.cpp
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Definition of the AttributeHistogram class
 */

#include "Debug.h"
#include "Enums.h"
#include "HistogramSettings.h"
#include "TransferFunctionHistogram.h"

TransferFunctionHistogram::TransferFunctionHistogram (QWidget* parent) :
    Histogram (parent)
{
    createActions ();
    SetDisplayColorBar (true);
}

void TransferFunctionHistogram::contextMenuEvent(QContextMenuEvent *event)
{
    m_pos = event->pos () -  canvas ()->geometry ().topLeft ();
    QMenu menu (this);
    menu.addAction (m_actionClampHigh.get ());
    menu.addAction (m_actionClampLow.get ());
    menu.addAction (m_actionClampClear.get ());
    menu.addAction (m_actionHeightSettings.get ());
    menu.exec (event->globalPos());
}


void TransferFunctionHistogram::createActions ()
{
    m_actionHeightSettings.reset (
	new QAction (tr("&Height Settings"), this));
    m_actionHeightSettings->setStatusTip(tr("Height Settings"));
    connect(m_actionHeightSettings.get (), SIGNAL(triggered()),
	    this, SLOT(HistogramSettingsDialog ()));
    
    m_actionClampHigh.reset (
	new QAction (tr("&Clamp High"), this));
    m_actionClampHigh->setStatusTip(tr("Clamp High"));
    connect(m_actionClampHigh.get (), SIGNAL(triggered()),
	    this, SLOT(SetClampMax ()));

    m_actionClampLow.reset (
	new QAction (tr("&Clamp Low"), this));
    m_actionClampLow->setStatusTip(tr("Clamp Low"));
    connect(m_actionClampLow.get (), SIGNAL(triggered()),
	    this, SLOT(SetClampMin ()));

    m_actionClampClear.reset (
	new QAction (tr("&Clamp Clear"), this));
    m_actionClampClear->setStatusTip(tr("Clamp Clear"));
    connect(m_actionClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(ClampClearSlot ()));
}


void TransferFunctionHistogram::SetClampMax ()
{
    double value = invTransform (xBottom, m_pos.x ());
    Q_EMIT SetClampMax (value);
}

void TransferFunctionHistogram::SetClampMin ()
{
    double value = invTransform (xBottom, m_pos.x ());
    Q_EMIT SetClampMin (value);
}

void TransferFunctionHistogram::ClampClearSlot ()
{
    SetAllItemsSelection (true);
    Q_EMIT ClampClear ();
}
