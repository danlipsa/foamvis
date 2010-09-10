/**
 * @file   AttributeHistogram.cpp
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Definition of the AttributeHistogram class
 */

#include "DebugStream.h"
#include "Enums.h"
#include "HistogramHeight.h"
#include "TransferFunctionHistogram.h"

TransferFunctionHistogram::TransferFunctionHistogram (QWidget* parent) :
    Histogram (parent)
{
    createActions ();
    SetDisplayColorBar (true);
}

void TransferFunctionHistogram::contextMenuEvent(QContextMenuEvent *event)
{
    m_pos = ToCanvas (event->pos ());
    qDebug () << event->pos ();
    qDebug () << m_pos;
    QMenu menu (this);
    menu.addAction (m_actionHeightSettings.get ());
    menu.addAction (m_actionClampHigh.get ());
    menu.addAction (m_actionClampLow.get ());
    menu.addAction (m_actionClampClear.get ());
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
	tr("&Clamp High"), this);
    m_actionClampHigh->setStatusTip(tr("Clamp High"));
    connect(m_actionClampHigh.get (), SIGNAL(triggered()),
	    this, SLOT(ClampHigh ()));

    m_actionClampLow = boost::make_shared<QAction> (
	tr("&Clamp Low"), this);
    m_actionClampLow->setStatusTip(tr("Clamp Low"));
    connect(m_actionClampLow.get (), SIGNAL(triggered()),
	    this, SLOT(ClampLow ()));

    m_actionClampClear = boost::make_shared<QAction> (
	tr("&Clamp Clear"), this);
    m_actionClampClear->setStatusTip(tr("Clamp Clear"));
    connect(m_actionClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(ClampClearSlot ()));
}


void TransferFunctionHistogram::ClampHigh ()
{
    double value = invTransform (xBottom, m_pos.x ());
    SetItemsSelectionHigh (false, value);
    Q_EMIT ClampHigh (value);
}

void TransferFunctionHistogram::ClampLow ()
{
    double value = invTransform (xBottom, m_pos.x ());
    SetItemsSelectionLow (false, value);
    Q_EMIT ClampLow (value);
}

void TransferFunctionHistogram::ClampClearSlot ()
{
    SetAllItemsSelection (true);
    Q_EMIT ClampClear ();
}
