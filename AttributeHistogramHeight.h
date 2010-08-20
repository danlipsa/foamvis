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
    AttributeHistogramHeight (QWidget* parent);

    size_t GetValue () const
    {
	return m_value;
    }
    void SetValue (size_t value);
    void SetMaximumValue (size_t currentTimeStepValue)
    {
	m_maximumValue = currentTimeStepValue;
    }
    size_t GetMaximumValue () const
    {
	return m_maximumValue;
    }

public Q_SLOTS:
    void ToggledMaximumValue (bool checked);
    void ToggledValue (bool checked);
    void EditingFinishedValue ();
    void FocusInValue ();

private:
    void setValue (size_t value);

private:
    Q_OBJECT
    QIntValidator m_validator;
    size_t m_value;
    size_t m_maximumValue;
};


#endif //__ATTRIBUTE_HISTOGRAM_HEIGHT_H__

// Local Variables:
// mode: c++
// End:
