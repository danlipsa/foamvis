/**
 * @file   Histogram.h
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Declaration of the Histogram class
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include "histogram_item.h"

class Histogram : public QwtPlot
{
public:
    enum SelectionTool
    {
	BRUSH,
	ERASER,
	COUNT
    };

public:
    Histogram (QWidget* parent = 0);
    void SetData (const QwtIntervalData& intervalData);
    void SetSelected (bool selected)
    {
	m_histogramItem.setSelected (selected);
    }
    void SetSelected (bool selected, size_t begin, size_t end)
    {
	m_histogramItem.setSelected (selected, begin, end);
    }
    void SetSelectionTool (SelectionTool selectionTool)
    {
	m_selectionTool = selectionTool;
    }
    void EnableSelection (bool enable = true);
    void GetSelection(vector<QwtDoubleInterval>* selection);

public Q_SLOTS:
    void SelectionPointMoved (const QPoint& pos);
    void SelectionPointAppended (const QPoint& pos);
    void PolygonSelected (const QwtPolygon& poly);

private:
    size_t getBin (float value);    

private:
    Q_OBJECT

    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
    QwtPlotPicker m_plotPicker;
    size_t m_beginBinSelection;
    SelectionTool m_selectionTool;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
