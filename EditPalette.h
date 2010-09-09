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
    EditPalette (QWidget* parent);
    void SetData (const QwtIntervalData& intervalData, double maxValue, 
		  const char* axisTitle, const QwtLinearColorMap& colorMap);
};

#endif //__EDIT_PALETTE_H__

// Local Variables:
// mode: c++
// End:
