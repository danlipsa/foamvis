/**
 * @file   Histogram.h
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Declaration of the Histogram class
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include "HistogramItem.h"
class HistogramHeight;

/**
 * Histogram that allows selection of bins.
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
    bool AreAllItemsSelected () const;
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
    void SetColorCoded (bool colorCoded = true)
    {
	m_histogramItem.setColorCoded (colorCoded);
    }
    void SetColorTransferFunction (const QwtDoubleInterval& interval, 
				   const QwtLinearColorMap& colorMap);
    void SetDisplayColorBar (bool displayColorBar = true);
    void SetMaxValueAxis (double axisMaxValue);
    void SetLogValueAxis (bool logValueAxis);
    void SetDataKeepBinSelection (
	const QwtIntervalData& intervalData, double maxValue,
	const char* axisTitle);
    void SetDataAllBinsSelected (
	const QwtIntervalData& intervalData, double maxValue, 
	const char* axisTitle);
    void SetAllItemsSelection (bool selected);
    void SetItemsSelectionHigh (bool selected, double value);
    void SetItemsSelectionLow (bool selected, double value);
    void SetSelectionTool (SelectionTool selectionTool);    
    bool IsGridEnabled () const;
    void SetDefaultFont ();
    virtual QSize sizeHint () const;
    void SetSizeHint (const QSize& sizeHint)
    {
	m_sizeHint = sizeHint;
    }

Q_SIGNALS:
    void SelectionChanged ();

public Q_SLOTS:
    void SelectionPointMoved (const QPoint& pos);
    void SelectionPointAppended (const QPoint& pos);
    void PolygonSelected (const QwtPolygon& poly);
    void HistogramHeightDialog ();
    void SetGridEnabled (bool enabled);



private:
    size_t getBin (double value);    
    void setData (const QwtIntervalData& intervalData, double maxValue,
		  const vector< pair<size_t, size_t> >* selectedBins = 0);
    void alignScales();
    void setAxisTitleDefaultFont (int axisId, const char* s = 0);
    void setAxisDefaultFont (int axisId);

private:
    Q_OBJECT

    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
    QwtPlotPicker m_plotPicker;
    size_t m_beginBinSelection;
    SelectionTool m_selectionTool;
    bool m_displayColorBar;
    boost::shared_ptr<HistogramHeight> m_histogramHeight;
    QSize m_sizeHint;

private:
    const static QSize SIZE_HINT;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
