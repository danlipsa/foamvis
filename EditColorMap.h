/**
 * @file   EditColorMap.h
 * @author Dan R. Lipsa
 * @date 23 August 2010
 * @brief Dialog to choose and clamp a color map.
 * @ingroup ui
 */

#ifndef __EDIT_COLOR_MAP_H__
#define __EDIT_COLOR_MAP_H__

#include "ui_EditColorMap.h"
#include "ColorBarModel.h"

/**
 * @brief Dialog to choose and clamp a color map.
 */
class EditColorMap : 
    public QDialog, private Ui::EditColorMap
{
public:
    EditColorMap (QWidget* parent);
    void SetData (const QwtIntervalData& histogram, double maxValue,
		  const ColorBarModel& colorBarModel, bool gridEnabled = true);
    
    const ColorBarModel& GetColorMap () const
    {
	return m_colorMap;
    }
    void SetDefaultFont ();

public Q_SLOTS:
    void HighlightedPalette (int index);
    void SetClampMax (double value);
    void SetClampMin (double value);
    void ClampClear ();
    void ToggledColorCodedHistogram (bool checked);
    void ToggledLog10Values (bool checked);
    void CurrentIndexChangedType (int i);
    void ClickedHighlight0 ();
    void ClickedHighlight1 ();
    void ClickedHighlight2 ();

private:
    void clickedHighlight (HighlightNumber::Enum highlightNumber);
    void setHighlightColors ();
    void setCombos (Palette palette);
    void fillCombo (PaletteSequential::Enum paletteSequential);
    void fillCombo (PaletteDiverging::Enum paletteDiverging);
    void setPalette (Palette palette);

private:
    static const char* HIGHLIGHT_LABEL_TEXT;

private:
    Q_OBJECT
    ColorBarModel m_colorMap;
    boost::array<QLabel*, HighlightNumber::COUNT> m_labelHighlight;
};

#endif //__EDIT_COLOR_MAP_H__

// Local Variables:
// mode: c++
// End:
