/**
 * @file   DisplayWidget.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the DisplayWidget class
 */

#include "DisplayWidget.h"
#include "DebugStream.h"


DisplayWidget::DisplayWidget (QWidget * parent) : 
    QWidget (parent), m_saveMovie(false), m_currentFrame(0)
{
}

void DisplayWidget::paintEvent (QPaintEvent * event)
{
    QWidget::paintEvent (event);
    saveFrame ();
}


void DisplayWidget::saveFrame ()
{
    if (m_saveMovie)
    {
        ostringstream file;
        file << "movie/frame" << setfill ('0') << setw (4) <<
	    m_currentFrame << ".jpg" << ends;
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

void DisplayWidget::ToggledSaveMovie (bool checked)
{
    cdbg << "Toggle save movie" << endl;
    m_saveMovie = checked;
    if (checked)
	m_currentFrame = 0;
    update ();
}
