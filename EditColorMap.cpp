/**
 * @file   EditColorMap.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the EditColorMap class
 */

#include "EditColorMap.h"
#include "Utils.h"

EditColorMap::EditColorMap (QWidget* parent) :    
    QDialog (parent)
{
    setupUi (this);
}

void EditColorMap::SetData (
    const QwtIntervalData& intervalData, double maxValue, const char* axisTitle,
    const QwtLinearColorMap& colorMap)
{
    widgetHistogram->SetDataAllBinsSelected (intervalData, maxValue, axisTitle);
    QwtDoubleInterval interval (
	intervalData.interval (0).minValue (),
	intervalData.interval (intervalData.size () - 1).maxValue ());
    widgetHistogram->SetColorMap (interval, colorMap);
}

void EditColorMap::HighlightedPalette (int index)
{
    VTK_CREATE(vtkColorTransferFunction, rainbow);
    rainbow->SetColorSpaceToHSV();
    rainbow->HSVWrapOff();
    rainbow->AddHSVPoint(0.0, 0.66667, 1.0, 1.0); // blue
    rainbow->AddHSVPoint(1.0, 0.0, 1.0, 1.0);     // red    
}
