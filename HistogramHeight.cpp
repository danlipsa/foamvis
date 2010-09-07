/**
 * @file   HistogramHeight.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the HistogramHeight class
 */

#include "HistogramHeight.h"

HistogramHeight::HistogramHeight (QWidget* parent) :    
    QDialog (parent), 
    m_validator (0, numeric_limits<int> ().max (), this),
    m_maxValue (0),
    m_state (VALUE)
{
    setupUi (this);
    lineEditValue->setValidator (&m_validator);
    setValue (0);
    
}

void HistogramHeight::ToggledMaxValue (bool checked)
{
    if (checked)
    {
	m_state = MAX_VALUE;
	setValue (m_maxValue);
    }
}

void HistogramHeight::ToggledValue (bool checked)
{
    if (checked)
    {
	m_state = VALUE;
	lineEditValue->setFocus ();
    }
}


void HistogramHeight::ToggledLogScale (bool checked)
{
    m_logScale = checked;
}


void HistogramHeight::EditingFinishedValue ()
{
    m_value = lineEditValue->text ().toInt();
}

void HistogramHeight::FocusInValue ()
{
    radioButtonValue->setChecked (true);
}

void HistogramHeight::setValue (size_t value)
{
    m_value = value;
    QString s;
    s.setNum (value);
    lineEditValue->setText (s);
}

void HistogramHeight::SetValue (size_t value)
{
    setValue (value);
    radioButtonValue->setChecked (true);
}

void HistogramHeight::SetLogScale (bool logScale)
{
    m_logScale = logScale;
    checkBoxLogScale->setChecked (logScale);
}
