/**
 * @file   Histogram.h
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Declaration of the TransferFunctionHistogram class
 */

#ifndef __TRANSFER_FUNCTION_HISTOGRAM_H__
#define __TRANSFER_FUNCTION_HISTOGRAM_H__

#include "Histogram.h"

class TransferFunctionHistogram : public Histogram
{
public:
    TransferFunctionHistogram (QWidget* parent = 0);

Q_SIGNALS:
    void ClampHigh (double value);
    void ClampLow (double value);
    void ClampClear ();

public Q_SLOTS:
    void ClampHigh ();
    void ClampLow ();
    void ClampClearSlot ();


protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void createActions ();

private:
    Q_OBJECT

    boost::shared_ptr<QAction> m_actionHeightSettings;
    boost::shared_ptr<QAction> m_actionClampHigh;
    boost::shared_ptr<QAction> m_actionClampLow;
    boost::shared_ptr<QAction> m_actionClampClear;
    QwtDoublePoint m_pos;
};

#endif //__TRANSFER_FUNCTION_HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
