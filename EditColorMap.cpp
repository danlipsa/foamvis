/**
 * @file   EditColorMap.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the EditColorMap class
 */

#include "ColorBarModel.h"
#include "Debug.h"
#include "EditColorMap.h"
#include "Utils.h"

EditColorMap::EditColorMap (
    QWidget* parent) :
    QDialog (parent)
{
    setupUi (this);
}

void EditColorMap::SetData (
    const QwtIntervalData& intervalData, double maxValue,
    const ColorBarModel& colorBarModel)
{
    m_colorBarModel = colorBarModel;
    comboBoxPalette->setCurrentIndex (m_colorBarModel.GetPalette ());
    widgetHistogram->SetDataAllBinsSelected (
	intervalData, maxValue, m_colorBarModel.GetTitle ().toAscii ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
}

void EditColorMap::HighlightedPalette (int index)
{
    RuntimeAssert ( index >= 0 && index <= Palette::LAST,
		    "Invalid palette index: ", index);
    m_colorBarModel.SetupPalette (
	static_cast<Palette::Enum> (index));
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
}

void EditColorMap::ClampHigh (double value)
{
    m_colorBarModel.SetClampHigh (value);
    m_colorBarModel.SetupPalette (m_colorBarModel.GetPalette ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
}

void EditColorMap::ClampLow (double value)
{
    m_colorBarModel.SetClampLow (value);
    m_colorBarModel.SetupPalette (m_colorBarModel.GetPalette ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
}

void EditColorMap::ClampClear ()
{
    m_colorBarModel.SetClampClear ();
    m_colorBarModel.SetupPalette (m_colorBarModel.GetPalette ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
}
