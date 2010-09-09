/**
 * @file   HistogramHeight.h
 * @author Dan R. Lipsa
 * @date 23 August 2010
 *
 * Declaration of the EditPalette class
 */

#ifndef __EDIT_PALETTE_H__
#define __EDIT_PALETTE_H__

#include "ui_EditPalette.h"

class EditPalette : 
    public QDialog, private Ui::EditPalette
{
public:
    enum Palettes
    {
	RAINBOW,
	BLACK_BODY,
	BLUE_TO_RED,
	BLUE_TO_TAN,
	PURPLE_TO_ORANGE,
	GREEN_TO_PURPLE,
	GREEN_TO_RED
    };
public:
    EditPalette (QWidget* parent);
    void SetData (const QwtIntervalData& intervalData, double maxValue, 
		  const char* axisTitle, const QwtLinearColorMap& colorMap);
public Q_SLOTS:
    void HighlightedPalette (int index);

private:
    Q_OBJECT
};

#endif //__EDIT_PALETTE_H__

// Local Variables:
// mode: c++
// End:
