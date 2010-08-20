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
    void SetData (const QwtIntervalData& intervalData, double maxValue,
		  const vector< pair<size_t, size_t> >* selectedBins = 0);
    void SetSelected (bool selected);
    void SetSelectionTool (SelectionTool selectionTool)
    {
	m_selectionTool = selectionTool;
    }
    void EnableSelection (bool enable = true);
    void GetSelectedIntervals(vector<QwtDoubleInterval>* intervals) const;
    void GetSelectedBins (
	vector< pair<size_t, size_t> >* intervals, bool selected = true) const
    {
	m_histogramItem.getSelectedBins (intervals, selected);
    }
    size_t GetAxisMaxValue () const
    {
	return m_axisMaxValue;
    }
    void SetAxisMaxValue (size_t axisMaxValue);
    size_t GetDataMaxValue () const;

public Q_SLOTS:
    void SelectionPointMoved (const QPoint& pos);
    void SelectionPointAppended (const QPoint& pos);
    void PolygonSelected (const QwtPolygon& poly);

Q_SIGNALS:
    void selectionChanged ();


private:
    size_t getBin (float value);    

private:
    Q_OBJECT

    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
    QwtPlotPicker m_plotPicker;
    size_t m_beginBinSelection;
    SelectionTool m_selectionTool;
    size_t m_axisMaxValue;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
