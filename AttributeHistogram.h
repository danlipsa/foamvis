/**
 * @file   AttributeHistogram.h
 * @author Dan R. Lipsa
 * @date 13 August 2010
 * @ingroup view
 * @brief A GUI histogram of a scalar attribute useful for one time step
 *        and all time steps.
 */

#ifndef __ATTRIBUTE_HISTOGRAM_H__
#define __ATTRIBUTE_HISTOGRAM_H__

#include "Histogram.h"
class ColorBarModel;

/**
 * @brief A GUI histogram of a scalar attribute useful for one time step
 *        and all time steps.
 */
class AttributeHistogram : public Histogram
{
public:
    AttributeHistogram (QWidget* parent = 0);
    void DisplayFocus (bool focus);

public Q_SLOTS:
    void SelectAll ();
    void DeselectAll ();
    void CurrentIndexChangedInteractionMode (int index);
    void SetColorTransferFunction (
	boost::shared_ptr<ColorBarModel> colorBarModel);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void createActions ();

private:
    Q_OBJECT

    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionHeightSettings;
};

#endif //__ATTRIBUTE_HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
