/**
 * @file   AttributeHistogramHeight.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the AttributeHistogramHeight class
 */

#include "AttributeHistogramHeight.h"

AttributeHistogramHeight::AttributeHistogramHeight (QWidget* parent) :
    QDialog (parent)
{
    setupUi (this);
}

void AttributeHistogramHeight::ToggledMaximumTimeSteps (bool checked)
{
    if (checked)
	m_height = MAXIMUM_TIME_STEPS;
}

void AttributeHistogramHeight::ToggledCurrentTimeStep (bool checked)
{
    if (checked)
	m_height = CURRENT_TIME_STEP;
}

void AttributeHistogramHeight::ToggleOtherValue (bool checked)
{
    if (checked)
	m_height = OTHER_VALUE;
}

void AttributeHistogramHeight::EditingFinishedOtherValue ()
{
}
