/**
 * @file   Histogram.h
 * @author Dan R. Lipsa
 * @date 13 August 2010
 *
 * Declaration of the AttributeHistogram class
 */

#ifndef __ATTRIBUTE_HISTOGRAM_H__
#define __ATTRIBUTE_HISTOGRAM_H__

#include "Histogram.h"

class AttributeHistogramHeight;

class AttributeHistogram : public Histogram
{
public:
    AttributeHistogram (QWidget* parent = 0);
    void SetActionSelectAll (boost::shared_ptr<QAction> actionSelectAll);
    void SetActionDeselectAll (boost::shared_ptr<QAction> actionDeselectAll);

public Q_SLOTS:
    void SelectAll ();
    void DeselectAll ();
    void HeightSettings ();
    void CurrentIndexChangedInteractionMode (int index);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void createActions ();

private:
    Q_OBJECT

    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionHeightSettings;
    boost::shared_ptr<AttributeHistogramHeight> m_attributeHistogramHeight;
};

#endif //__ATTRIBUTE_HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
