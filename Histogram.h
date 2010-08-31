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
	NONE,
	BRUSH,
	ERASER	
    };

public:
    Histogram (QWidget* parent = 0);
    double GetMaxValueAxis () const
    {
	return m_histogramItem.getMaxValueAxis ();
    }
    size_t GetMaxValueData () const;
    double GetMinValueAxis () const
    {
	return m_histogramItem.getMinValueAxis ();
    }
    void GetSelectedIntervals(vector<QwtDoubleInterval>* intervals) const;
    void GetSelectedBins (
	vector< pair<size_t, size_t> >* intervals, bool selected = true) const
    {
	m_histogramItem.getSelectedBins (intervals, selected);
    }
    bool IsLogValueAxis () const
    {
	return m_histogramItem.isLogValueAxis ();
    }
    void SetColorCoded (bool enable = true)
    {
	m_histogramItem.setColorCoded (enable);
    }
    void SetColorMap (const QwtLinearColorMap& colorMap)
    {
	m_histogramItem.setColorMap (colorMap);
    }
    void SetMaxValueAxis (double axisMaxValue);
    void SetLogValueAxis (bool logValueAxis);
    void SetDataKeepBinSelection (
	const QwtIntervalData& intervalData, double maxValue);
    void SetDataAllBinsSelected (
	const QwtIntervalData& intervalData, double maxValue);
    void SetAllItemsSelection (bool selected);
    void SetSelectionTool (SelectionTool selectionTool);    

public Q_SLOTS:
    void SelectionPointMoved (const QPoint& pos);
    void SelectionPointAppended (const QPoint& pos);
    void PolygonSelected (const QwtPolygon& poly);

Q_SIGNALS:
    void selectionChanged ();


private:
    size_t getBin (float value);    
    void setData (const QwtIntervalData& intervalData, double maxValue,
		  const vector< pair<size_t, size_t> >* selectedBins = 0);


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
