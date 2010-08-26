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
    enum State
    {
	MAX_VALUE,
	VALUE
    };

public:
    AttributeHistogramHeight (QWidget* parent);

    size_t GetValue () const
    {
	return m_value;
    }
    void SetValue (size_t value);
    void SetMaximumValue (size_t maxValue)
    {
	m_maxValue = maxValue;
    }
    size_t GetMaximumValue () const
    {
	return m_maxValue;
    }
    State GetState () const
    {
	return m_state;
    }
    bool IsLogScale () const
    {
	return m_logScale;
    }
    void SetLogScale (bool logScale);

public Q_SLOTS:
    void ToggledLogScale (bool checked);
    void ToggledMaxValue (bool checked);
    void ToggledValue (bool checked);
    void EditingFinishedValue ();
    void FocusInValue ();

private:
    void setValue (size_t value);

private:
    Q_OBJECT
    QIntValidator m_validator;
    size_t m_value;
    size_t m_maxValue;
    State m_state;
    bool m_logScale;
};


#endif //__ATTRIBUTE_HISTOGRAM_HEIGHT_H__

// Local Variables:
// mode: c++
// End:
