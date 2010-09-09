/**
 * @file   HistogramHeight.h
 * @author Dan R. Lipsa
 * @date 23 August 2010
 *
 * Declaration of the EditTransferFunction class
 */

#ifndef __EDIT_TRANSFER_FUNCTION_H__
#define __EDIT_TRANSFER_FUNCTION_H__

#include "ui_EditPalette.h"

class EditTransferFunction : 
    public QDialog, private Ui::EditPalette
{
public:
    EditTransferFunction (QWidget* parent);
    void SetData (const QwtIntervalData& intervalData, double maxValue, 
		  const char* axisTitle, const QwtLinearColorMap& colorMap);
};

#endif //__EDIT_TRANSFER_FUNCTION_H__

// Local Variables:
// mode: c++
// End:
