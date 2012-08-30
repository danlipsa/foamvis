/**
 * @file   WidgetHistogram.h
 * @author Dan R. Lipsa
 * @date 24 Aug. 2012
 *
 * Widget for displaying histograms
 */
#ifndef __WIDGET_HISTOGRAM_H__
#define __WIDGET_HISTOGRAM_H__

#include "WidgetBase.h"
class AttributeHistogram;
class SimulationGroup;
class ColorBarModel;

/**
 * Widget for displaying histograms
 */
class WidgetHistogram : public QWidget, public WidgetBase
{
public:
    enum SelectionOperation
    {
	KEEP_SELECTION,
	DISCARD_SELECTION
    };

    enum MaxValueOperation
    {
	KEEP_MAX_VALUE,
	REPLACE_MAX_VALUE
    };

public:
    WidgetHistogram (QWidget* parent = 0);
    void Init (boost::shared_ptr<Settings> settings, 
	       const SimulationGroup* simulationGroup);
    int GetHeight () const;
    void UpdateFocus ();
    void UpdateSelection (ViewNumber::Enum viewNumber);
    void Update (boost::shared_ptr<ColorBarModel> colorBarModel,
		 SelectionOperation selectionOperation,
		 MaxValueOperation maxValueOperation);
    void Update (boost::shared_ptr<ColorBarModel> colorBarModel,
		 SelectionOperation selectionOperation,
		 MaxValueOperation maxValueOperation,
		 ViewNumber::Enum viewNumber);
    void UpdateHidden ();
    void CurrentIndexChangedInteractionMode (int index);
    void SetHeight (int h);
    void SetGridShown (bool shown);
    void SetDefaultFont ();
    AttributeHistogram& GetHistogram (size_t i)
    {
	return *m_histogram[i];
    }

    virtual const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;

Q_SIGNALS:
    void SelectionChanged (int viewNumber);

protected:
    virtual void mousePressEvent(QMouseEvent *event);

private Q_SLOTS:
    void selectionChanged (int viewNumber);

private:
    void updateFocus (ViewNumber::Enum viewNumber);
    void updateHidden (ViewNumber::Enum viewNumber);
    void currentIndexChangedInteractionMode (
	ViewNumber::Enum viewNumber, int index);
    void setHeight (ViewNumber::Enum viewNumber, int h);
    void setGridShown (ViewNumber::Enum viewNumber, bool shown);
    void setDefaultFont (ViewNumber::Enum viewNumber);
    void setView (ViewNumber::Enum viewNumber, QWidget* widget);

private:
    Q_OBJECT
    boost::array<AttributeHistogram*, ViewNumber::COUNT> m_histogram;    
    const SimulationGroup* m_simulationGroup;
};


#endif //__WIDGET_HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
