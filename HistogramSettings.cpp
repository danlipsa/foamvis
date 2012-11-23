/**
 * @file   HistogramSettings.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the HistogramSettings class
 */

#include "HistogramSettings.h"

HistogramSettings::HistogramSettings (QWidget* parent) :    
    QDialog (parent), 
    m_yAxisLogScale (false),
    m_validator (0, numeric_limits<int> ().max (), this),
    m_yAxisMaxValue (0),
    m_yState (VALUE)
{
    setupUi (this);
    lineEditValue->setValidator (&m_validator);
    setValue (0);
    
}

void HistogramSettings::ToggledYMaxValue (bool checked)
{
    if (checked)
    {
	m_yState = MAX_VALUE;
	setValue (m_yAxisMaxValue);
    }
}

void HistogramSettings::ToggledYValue (bool checked)
{
    if (checked)
    {
	m_yState = VALUE;
	lineEditValue->setFocus ();
    }
}


void HistogramSettings::ToggledYAxisLogScale (bool checked)
{
    m_yAxisLogScale = checked;
}


void HistogramSettings::EditingFinishedYValue ()
{
    m_yAxisValue = lineEditValue->text ().toInt();
}

void HistogramSettings::FocusInYValue ()
{
    radioButtonYValue->setChecked (true);
}

void HistogramSettings::setValue (size_t value)
{
    m_yAxisValue = value;
    QString s;
    s.setNum (value);
    lineEditValue->setText (s);
}

void HistogramSettings::SetYValue (size_t value)
{
    setValue (value);
    radioButtonYValue->setChecked (true);
}

void HistogramSettings::SetYAxisLogScale (bool logScale)
{
    m_yAxisLogScale = logScale;
    checkBoxYAxisLogScale->setChecked (logScale);
}

