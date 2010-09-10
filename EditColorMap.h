/**
 * @file   HistogramHeight.h
 * @author Dan R. Lipsa
 * @date 23 August 2010
 *
 * Declaration of the EditColorMap class
 */

#ifndef __EDIT_PALETTE_H__
#define __EDIT_PALETTE_H__

#include "ui_EditColorMap.h"

class EditColorMap : 
    public QDialog, private Ui::EditColorMap
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
    EditColorMap (QWidget* parent);
    void SetData (const QwtIntervalData& intervalData, double maxValue, 
		  const char* axisTitle, const QwtLinearColorMap& colorMap);
public Q_SLOTS:
    void HighlightedPalette (int index);

private:

private:
    Q_OBJECT
};

#endif //__EDIT_PALETTE_H__

// Local Variables:
// mode: c++
// End:
