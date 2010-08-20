/**
 * @file   AttributeHistogramHeight.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the AttributeHistogramHeight class
 */

#include "AttributeHistogramHeight.h"

AttributeHistogramHeight::AttributeHistogramHeight (QWidget* parent) :    
    QDialog (parent), 
    m_validator (0, numeric_limits<int> ().max (), this),
    m_maximumValue (0)
{
    setupUi (this);
    lineEditValue->setValidator (&m_validator);
    setValue (0);
    
}

void AttributeHistogramHeight::ToggledMaximumValue (bool checked)
{
    if (checked)
	setValue (m_maximumValue);
}

void AttributeHistogramHeight::ToggledValue (bool checked)
{
    if (checked)
	lineEditValue->setFocus ();
}

void AttributeHistogramHeight::EditingFinishedValue ()
{
    m_value = lineEditValue->text ().toInt();
}

void AttributeHistogramHeight::FocusInValue ()
{
    radioButtonValue->setChecked (true);
}

void AttributeHistogramHeight::setValue (size_t value)
{
    m_value = value;
    QString s;
    s.setNum (value);
    lineEditValue->setText (s);
}

void AttributeHistogramHeight::SetValue (size_t value)
{
    setValue (value);
    radioButtonValue->setChecked (true);
}
