/**
 * @file   Application.h
 * @author Dan R. Lipsa
 * @date 27 August 2010
 *
 * Declaration of the Application class
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

class Application: public QApplication
{
public:
    Application(int& c, char** v) :
	QApplication(c, v) 
    {
    }
    virtual bool notify(QObject *rec, QEvent *ev);    

public:
    static boost::shared_ptr<Application> Get (int& c, char** v);
    static boost::shared_ptr<Application> Get ();

private:
    static boost::shared_ptr<Application> m_application;
};



#endif //__APPLICATION_H__

// Local Variables:
// mode: c++
// End:
