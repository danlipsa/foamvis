/**
 * @file   Info.h
 * @author Dan R. Lipsa
 * @date 18 October 2010
 *
 * Declaration of the Info class
 */

#ifndef __INFO_H__
#define __INFO_H__

#include "ui_Info.h"

class Info : 
    public QDialog, private Ui::Info
{

public:
    Info (QWidget* parent, const string& str) :
	QDialog (parent)
    {
	setupUi (this);
	text->setPlainText (str.c_str ());
    }
};


#endif //__INFO_H__

// Local Variables:
// mode: c++
// End:
