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
    Info (QWidget* parent, const char* title, const char* text = 0) :
	QDialog (parent)
    {
	setupUi (this);
	setWindowTitle (title);
	if (text != 0)
	    textEdit->setPlainText (text);
    }
    
    void setText (const char* text)
    {
	textEdit->setPlainText (text);
    }
};


#endif //__INFO_H__

// Local Variables:
// mode: c++
// End:
