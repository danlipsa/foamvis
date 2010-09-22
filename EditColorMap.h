/**
 * @file   HistogramHeight.h
 * @author Dan R. Lipsa
 * @date 23 August 2010
 *
 * Declaration of the EditColorMap class
 */

#ifndef __EDIT_COLOR_MAP_H__
#define __EDIT_COLOR_MAP_H__

#include "ui_EditColorMap.h"
#include "ColorBarModel.h"

class EditColorMap : 
    public QDialog, private Ui::EditColorMap
{
public:
    EditColorMap (QWidget* parent);
    void SetData (const QwtIntervalData& histogram, double maxValue,
		  const ColorBarModel& colorBarModel);

    const ColorBarModel& GetColorBarModel () const
    {
	return m_colorBarModel;
    }

public Q_SLOTS:
    void HighlightedPalette (int index);
    void ClampHigh (double value);
    void ClampLow (double value);
    void ClampClear ();
    void ToggledColorCodedHistogram(bool checked);

private:

private:
    Q_OBJECT
    ColorBarModel m_colorBarModel;
};

#endif //__EDIT_COLOR_MAP_H__

// Local Variables:
// mode: c++
// End:
