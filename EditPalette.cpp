/**
 * @file   EditPalette.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the EditPalette class
 */

#include "EditPalette.h"

EditPalette::EditPalette (QWidget* parent) :    
    QDialog (parent)
{
    setupUi (this);
}

void EditPalette::SetData (
    const QwtIntervalData& intervalData, double maxValue, const char* axisTitle,
    const QwtLinearColorMap& colorMap)
{
    widgetHistogram->SetDataAllBinsSelected (intervalData, maxValue, axisTitle);
    QwtDoubleInterval interval (
	intervalData.interval (0).minValue (),
	intervalData.interval (intervalData.size () - 1).maxValue ());
    widgetHistogram->SetColorMap (interval, colorMap);
}
