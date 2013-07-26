/**
 * @file   WidgetSave.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the WidgetSave class
 */

#include "WidgetSave.h"
#include "Debug.h"


WidgetSave::WidgetSave (QWidget * parent) : 
    QWidget (parent), 
    m_saveMovie(false), 
    m_currentFrame(0)
{
}

void WidgetSave::paintEvent ( QPaintEvent * event )
{
    static_cast<void> (event);
    SaveFrame ();
}

void WidgetSave::SaveFrame ()
{
    if (m_saveMovie)
    {
        ostringstream file;
        file << "movie/frame" << setfill ('0') << setw (4) <<
	    m_currentFrame << ".png" << ends;
	cdbg << "Taking snapshot ...";
        QImage snapshot = 
	    QPixmap::grabWindow (winId ()).toImage ();
	string f = file.str ();
	cdbg << "Saving " << f << " ..." << endl;
	if (! snapshot.save (f.c_str ()))
	    cdbg << "Error saving " << f << endl;
	m_currentFrame++;
    }    
}

void WidgetSave::ToggledSaveMovie (bool checked)
{
    m_saveMovie = checked;
    if (checked)
	m_currentFrame = 0;
    update ();
}

QSize WidgetSave::sizeHint () const
{
    return QSize (720, 480);
}

QSize WidgetSave::minimumSizeHint () const
{
    return QSize (360, 240);
}
