/**
 * @file   AttributeHistogram.cpp
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Definition of the AttributeHistogram class
 */

#include "TransferFunctionHistogram.h"
#include "HistogramHeight.h"
#include "Enums.h"

TransferFunctionHistogram::TransferFunctionHistogram (QWidget* parent) :
    Histogram (parent)
{
    createActions ();
    SetDisplayColorBar (true);
}

void TransferFunctionHistogram::contextMenuEvent(QContextMenuEvent *event)
{
    m_pos = event->pos ();
    QMenu menu (this);
    menu.addAction (m_actionHeightSettings.get ());
    menu.addAction (m_actionClampHigh.get ());
    menu.addAction (m_actionClampLow.get ());
    menu.exec (event->globalPos());
}


void TransferFunctionHistogram::createActions ()
{
    m_actionHeightSettings = boost::make_shared<QAction> (
	tr("&Height Settings"), this);
    m_actionHeightSettings->setStatusTip(tr("Height Settings"));
    connect(m_actionHeightSettings.get (), SIGNAL(triggered()),
	    this, SLOT(HistogramHeightDialog ()));

    m_actionClampHigh = boost::make_shared<QAction> (
	tr("&Clamp Pallete High"), this);
    m_actionClampHigh->setStatusTip(tr("Clamp Pallete High"));
    connect(m_actionClampHigh.get (), SIGNAL(triggered()),
	    this, SLOT(ClampHigh ()));

    m_actionClampLow = boost::make_shared<QAction> (
	tr("&Clamp Pallete Low"), this);
    m_actionClampLow->setStatusTip(tr("Clamp Pallete Low"));
    connect(m_actionClampLow.get (), SIGNAL(triggered()),
	    this, SLOT(ClampLow ()));
}


void TransferFunctionHistogram::ClampHigh ()
{
    
}

void TransferFunctionHistogram::ClampLow ()
{
}

