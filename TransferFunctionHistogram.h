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


/**
 * Histogram that allows clamping low and high.
 */
class TransferFunctionHistogram : public Histogram
{
public:
    TransferFunctionHistogram (QWidget* parent = 0);

Q_SIGNALS:
    void SetClampMax (double value);
    void SetClampMin (double value);
    void ClampClear ();

public Q_SLOTS:
    void SetClampMax ();
    void SetClampMin ();
    void ClampClearSlot ();


protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void createActions ();

private:
    Q_OBJECT

    boost::scoped_ptr<QAction> m_actionHeightSettings;
    boost::scoped_ptr<QAction> m_actionClampHigh;
    boost::scoped_ptr<QAction> m_actionClampLow;
    boost::scoped_ptr<QAction> m_actionClampClear;
    QwtDoublePoint m_pos;
};

#endif //__TRANSFER_FUNCTION_HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
