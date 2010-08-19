/**
 * @file   AttributeHistogramHeight.h
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Declaration of the AttributeHistogramHeight class
 */

#ifndef __ATTRIBUTE_HISTOGRAM_HEIGHT_H__
#define __ATTRIBUTE_HISTOGRAM_HEIGHT_H__

#include "ui_AttributeHistogramHeight.h"

class AttributeHistogramHeight : 
    public QDialog, private Ui::AttributeHistogramHeight
{
public:
    enum Height
    {
	MAXIMUM_TIME_STEPS,
	CURRENT_TIME_STEP,
	OTHER_VALUE
    };

public:
    AttributeHistogramHeight (QWidget* parent);

    Height GetHeight () const
    {
	return m_height;
    }

    double GetOtherValue () const
    {
	return m_otherValue;
    }

public Q_SLOTS:
    void ToggledMaximumTimeSteps (bool checked);
    void ToggledCurrentTimeStep (bool checked);
    void ToggleOtherValue (bool checked);
    void EditingFinishedOtherValue ();

private:
    Q_OBJECT
    Height m_height;
    double m_otherValue;
};

#endif //__ATTRIBUTE_HISTOGRAM_HEIGHT_H__

// Local Variables:
// mode: c++
// End:
