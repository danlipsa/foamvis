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

const char* EditColorMap::HIGHLIGHT_LABEL_TEXT = "Hightlight";

EditColorMap::EditColorMap (
    QWidget* parent) :
    QDialog (parent)
{
    setupUi (this);
    m_labelHighlight[0] = labelHighlight0;
    m_labelHighlight[1] = labelHighlight1;
    m_labelHighlight[2] = labelHighlight2;
}

void EditColorMap::setCombos (Palette palette)
{
    comboBoxType->setCurrentIndex (palette.m_type);
    comboBoxPalette->clear ();
    if (palette.m_type == PaletteType::SEQUENTIAL)
	fillCombo (palette.m_sequential);
    else
	fillCombo (palette.m_diverging);
}

void EditColorMap::fillCombo (PaletteSequential::Enum paletteSequential)
{
    for (size_t i = 0; i < PaletteSequential::COUNT; i++)
	comboBoxPalette->insertItem (
	    i, PaletteSequential::ToString (PaletteSequential::Enum (i)));
    comboBoxPalette->setCurrentIndex (paletteSequential);
}

void EditColorMap::fillCombo (PaletteDiverging::Enum paletteDiverging)
{
    for (size_t i = 0; i < PaletteDiverging::COUNT; i++)
	comboBoxPalette->insertItem (
	    i, PaletteDiverging::ToString (PaletteDiverging::Enum (i)));
    comboBoxPalette->setCurrentIndex (paletteDiverging);
}


void EditColorMap::SetData (
    const QwtIntervalData& intervalData, double maxValue,
    const ColorBarModel& colorBarModel, bool gridEnabled)
{
    m_colorBarModel = colorBarModel;
    setCombos (m_colorBarModel.GetPalette ());
    widgetHistogram->SetDataAllBinsSelected (
	intervalData, maxValue, 
	m_colorBarModel.GetTitle ().toStdString ().c_str ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
    widgetHistogram->SetGridEnabled (gridEnabled);
    QwtDoubleInterval interval = m_colorBarModel.GetInterval ();
    QwtDoubleInterval clampValues = m_colorBarModel.GetClampValues ();
    if (clampValues.minValue () > interval.minValue ())
	widgetHistogram->SetItemsSelectionLow (false, clampValues.minValue ());
    if (clampValues.maxValue () < interval.maxValue ())
	widgetHistogram->SetItemsSelectionHigh (false, clampValues.maxValue ());
    setHighlightColors ();
 }

void EditColorMap::setHighlightColors ()
{
    for (size_t i = 0; i < m_labelHighlight.size (); ++i)
	m_labelHighlight[i]->setText (
	    ColorToHtml (m_colorBarModel.GetHighlightColor (
			     HighlightNumber::Enum (i)), 
			 HIGHLIGHT_LABEL_TEXT).c_str ());
}


void EditColorMap::clickedHighlight (HighlightNumber::Enum highlightNumber)
{
    QColor color = QColorDialog::getColor (m_colorBarModel.GetHighlightColor (
					       highlightNumber));
    if (color.isValid ())
    {
	m_colorBarModel.SetHighlightColor (highlightNumber, color);
	m_labelHighlight[highlightNumber]->setText (
	    ColorToHtml (color, HIGHLIGHT_LABEL_TEXT).c_str ());
    }
}

void EditColorMap::SetDefaultFont ()
{
    widgetHistogram->SetDefaultFont ();
}

void EditColorMap::setPalette (Palette palette)
{
    m_colorBarModel.SetupPalette (palette);
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
    widgetHistogram->replot ();
    setHighlightColors ();    
}

// SLOTS
// ======================================================================
void EditColorMap::CurrentIndexChangedType (int i)
{
    Palette palette = m_colorBarModel.GetPalette ();
    Palette newPalette = Palette (PaletteType::Enum (i), palette.m_sequential,
				  palette.m_diverging);
    setCombos (newPalette);
    setPalette (newPalette);
}

void EditColorMap::HighlightedPalette (int index)
{
    setPalette (Palette (m_colorBarModel.GetPalette ().m_type, index));
}

void EditColorMap::ToggledColorCodedHistogram(bool checked)
{
    widgetHistogram->SetColorCoded (checked);
    widgetHistogram->replot ();
}

void EditColorMap::ClickedHighlight0 ()
{
    clickedHighlight (HighlightNumber::H0);
}

void EditColorMap::ClickedHighlight1 ()
{
    clickedHighlight (HighlightNumber::H1);
}

void EditColorMap::ClickedHighlight2 ()
{
    clickedHighlight (HighlightNumber::H2);
}

void EditColorMap::ClampHigh (double value)
{
    m_colorBarModel.SetClampHigh (value);
    m_colorBarModel.SetupPalette (m_colorBarModel.GetPalette ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
    widgetHistogram->SetItemsSelectionHigh (false, value);
}

void EditColorMap::ClampLow (double value)
{
    m_colorBarModel.SetClampLow (value);
    m_colorBarModel.SetupPalette (m_colorBarModel.GetPalette ());
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
    widgetHistogram->SetItemsSelectionLow (false, value);
}

void EditColorMap::ClampClear ()
{
    m_colorBarModel.SetClampClear ();
    widgetHistogram->SetColorMap (
	m_colorBarModel.GetInterval (), m_colorBarModel.GetColorMap ());
}
