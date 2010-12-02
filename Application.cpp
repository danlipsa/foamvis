/**
 * @file   Application.cpp
 * @author Dan R. Lipsa
 * @date 27 August 2010
 *
 * Definition of the Application class
 */

#include "Application.h"
#include "Debug.h"
#include "DebugStream.h"

boost::shared_ptr<Application> Application::m_application;


Application::Application(int& c, char** v) :
    QApplication(c, v) 
{
    QFont defaultFont = font ();
    defaultFont.setPointSize (8);
    setFont (defaultFont);
}



boost::shared_ptr<Application> Application::Get (int& c, char** v)
{
    if (m_application == 0)
	m_application.reset (new Application (c, v));
    return m_application;
}

boost::shared_ptr<Application> Application::Get ()
{
    if (m_application == 0)
	ThrowException ("The application object have to be created first");
    return m_application;
}

void Application::release ()
{
    m_application.reset ();
}

bool Application::notify(QObject *rec, QEvent *ev)
{
    try
    {
	return QApplication::notify(rec, ev);
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
	return false;
    }
}
