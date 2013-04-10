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
    m_colorMap = colorBarModel;
    setCombos (m_colorMap.GetPalette ());
    widgetHistogram->SetDataAllBinsSelected (
	intervalData, maxValue, 
	m_colorMap.GetTitle ().c_str ());
    widgetHistogram->SetColorTransferFunction (
	m_colorMap.GetInterval (), m_colorMap.GetQwtColorMap ());
    widgetHistogram->SetGridEnabled (gridEnabled);
    QwtDoubleInterval interval = m_colorMap.GetInterval ();
    QwtDoubleInterval clampValues = m_colorMap.GetClampInterval ();
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
	    ColorToHtml (m_colorMap.GetHighlightColor (
			     HighlightNumber::Enum (i)), 
			 HIGHLIGHT_LABEL_TEXT).c_str ());
}


void EditColorMap::clickedHighlight (HighlightNumber::Enum highlightNumber)
{
    QColor color = QColorDialog::getColor (m_colorMap.GetHighlightColor (
					       highlightNumber));
    if (color.isValid ())
    {
	m_colorMap.SetHighlightColor (highlightNumber, color);
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
    m_colorMap.SetupPalette (palette);
    widgetHistogram->SetColorTransferFunction (
	m_colorMap.GetInterval (), m_colorMap.GetQwtColorMap ());
    widgetHistogram->replot ();
    setHighlightColors ();    
}

// SLOTS
// ======================================================================
void EditColorMap::CurrentIndexChangedType (int i)
{
    Palette palette = m_colorMap.GetPalette ();
    Palette newPalette = Palette (PaletteType::Enum (i), palette.m_sequential,
				  palette.m_diverging);
    setCombos (newPalette);
    setPalette (newPalette);
}

void EditColorMap::HighlightedPalette (int index)
{
    setPalette (Palette (m_colorMap.GetPalette ().m_type, index));
}

void EditColorMap::ToggledColorCodedHistogram(bool checked)
{
    widgetHistogram->SetColorCoded (checked);
    widgetHistogram->replot ();
}

void EditColorMap::ToggledLog10Values (bool checked)
{
    m_colorMap.SetLog10 (checked);
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

void EditColorMap::SetClampMax (double value)
{
    m_colorMap.SetClampMax (value);
    widgetHistogram->SetColorTransferFunction (
	m_colorMap.GetInterval (), m_colorMap.GetQwtColorMap ());
    widgetHistogram->SetItemsSelectionHigh (false, value);
}

void EditColorMap::SetClampMin (double value)
{
    m_colorMap.SetClampMin (value);
    widgetHistogram->SetColorTransferFunction (
	m_colorMap.GetInterval (), m_colorMap.GetQwtColorMap ());
    widgetHistogram->SetItemsSelectionLow (false, value);
}

void EditColorMap::ClampClear ()
{
    m_colorMap.SetClampClear ();
    widgetHistogram->SetColorTransferFunction (
	m_colorMap.GetInterval (), m_colorMap.GetQwtColorMap ());
}
