/**
 * @file   Histogram.h
 * @author Dan R. Lipsa
 * @date 22 July 2010
 * @brief A histogram UI that allows selection of bins.
 * @ingroup views
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__
#include "Enums.h"

class HistogramItem;
class HistogramSettings;

/**
 * @brief A histogram UI that allows selection of bins.
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
    void GetSelectedIntervals(vector<QwtDoubleInterval>* intervals) const;
    void GetSelectedBins (BinRegions* intervals, bool selected = true) const;
    void SetSelectedBinsNoSignal (const BinRegions& bins);
    void SetColorCoded (bool colorCoded);
    void SetColorTransferFunction (const QwtDoubleInterval& interval, 
				   const QwtLinearColorMap& colorMap);
    void SetDisplayColorBar (bool displayColorBar = true);
    void SetYAxisLogScale (bool logYAxis);
    bool IsYAxisLogScale () const;
    void SetYAxisMaxValue (double axisMaxValue);
    double GetYAxisMaxValue () const;
    size_t GetYAxisMaxValueData () const;
    double GetYAxisMinValue () const;

    void SetXAxisLogScale (bool logYAxis);
    void SetXAxisMaxValue (double value);
    double GetXAxisMaxValue () const;
    void SetXAxisMinValue (double value);
    double GetXAxisMinValue () const;
    void SetDataKeepBinSelection (
	const QwtIntervalData& intervalData, double maxValue,
	const char* axisTitle, const BinRegions& selectedBins);
    void SetDataAllBinsSelected (
	const QwtIntervalData& intervalData, double maxValue, 
	const char* axisTitle);
    bool HasData () const;
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
    void HistogramSettingsDialog ();
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
    boost::shared_ptr<HistogramItem> m_histogramItem;
    QwtPlotPicker m_plotPicker;
    size_t m_beginBinSelection;
    SelectionTool m_selectionTool;
    bool m_displayColorBar;
    boost::shared_ptr<HistogramSettings> m_histogramHeight;
    QSize m_sizeHint;

private:
    const static QSize SIZE_HINT;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
