/**
 * @file   EditTransferFunction.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the EditTransferFunction class
 */

#include "EditTransferFunction.h"

EditTransferFunction::EditTransferFunction (QWidget* parent) :    
    QDialog (parent)
{
    setupUi (this);
}

void EditTransferFunction::SetData (
    const QwtIntervalData& intervalData, double maxValue, const char* axisTitle,
    const QwtLinearColorMap& colorMap)
{
    widgetHistogram->SetDataAllBinsSelected (intervalData, maxValue, axisTitle);
    QwtDoubleInterval interval (
	intervalData.interval (0).minValue (),
	intervalData.interval (intervalData.size () - 1).maxValue ());
    widgetHistogram->SetColorMap (interval, colorMap);
}
