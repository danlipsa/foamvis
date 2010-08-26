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
    m_maxValue (0),
    m_state (VALUE)
{
    setupUi (this);
    lineEditValue->setValidator (&m_validator);
    setValue (0);
    
}

void AttributeHistogramHeight::ToggledMaxValue (bool checked)
{
    if (checked)
    {
	m_state = MAX_VALUE;
	setValue (m_maxValue);
    }
}

void AttributeHistogramHeight::ToggledValue (bool checked)
{
    if (checked)
    {
	m_state = VALUE;
	lineEditValue->setFocus ();
    }
}


void AttributeHistogramHeight::ToggledLogScale (bool checked)
{
    m_logScale = checked;
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

void AttributeHistogramHeight::SetLogScale (bool logScale)
{
    m_logScale = logScale;
    checkBoxLogScale->setChecked (logScale);
}
