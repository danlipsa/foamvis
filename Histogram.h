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

/**
 * Histogram that allows selection of bins.
 * @todo add logarithmic scale for values of the histogram.
 */
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
    double GetMaxValueAxis () const
    {
	return m_histogramItem.getMaxValueAxis ();
    }
    double GetMinValueAxis () const;
    size_t GetMaxValueData () const;

    void SetMaxValueAxis (double axisMaxValue);
    void SetLogValueAxis (bool logValueAxis);
    bool IsLogValueAxis () const
    {
	return m_logValueAxis;
    }

public Q_SLOTS:
    void SelectionPointMoved (const QPoint& pos);
    void SelectionPointAppended (const QPoint& pos);
    void PolygonSelected (const QwtPolygon& poly);

Q_SIGNALS:
    void selectionChanged ();


private:
    size_t getBin (float value);    

private:
    static const double logScaleZero;

private:
    Q_OBJECT

    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
    QwtPlotPicker m_plotPicker;
    size_t m_beginBinSelection;
    SelectionTool m_selectionTool;
    bool m_logValueAxis;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
