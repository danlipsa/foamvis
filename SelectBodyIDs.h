/**
 * @file   SelectBodyIDs.h
 * @author Dan R. Lipsa
 * @date 4 Dec. 2010
 *
 * Declaration of the SelectBodyIDs
 */

#ifndef __EDIT_BODY_IDS_H__
#define __EDIT_BODY_IDS_H__

#include "ui_SelectBodyIDs.h"
#include "ColorBarModel.h"

class SelectBodyIDs : public QDialog, private Ui::SelectBodyIDs
{
public:
    SelectBodyIDs (QWidget* parent);

private:
    Q_OBJECT
    ColorBarModel m_colorBarModel;
};

#endif //__EDIT_BODY_IDS_H__

// Local Variables:
// mode: c++
// End:
