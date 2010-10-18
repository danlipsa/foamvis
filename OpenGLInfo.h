/**
 * @file   OpenGLInfo.h
 * @author Dan R. Lipsa
 * @date 18 October 2010
 *
 * Declaration of the OpenGLInfo class
 */

#ifndef __OPENGL_INFO_H__
#define __OPENGL_INFO_H__

#include "ui_OpenGLInfo.h"

class OpenGLInfo : 
    public QDialog, private Ui::OpenGLInfo
{

public:
    OpenGLInfo (QWidget* parent, const string& str) :
	QDialog (parent)
    {
	setupUi (this);
	text->setPlainText (str.c_str ());
    }
};


#endif //__OPENGL_INFO_H__

// Local Variables:
// mode: c++
// End:
