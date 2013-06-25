/**
 * @file   Application.h
 * @author Dan R. Lipsa
 * @date 27 August 2010
 * @ingroup ui
 * @brief Implements the singleton design pattern to access a global 
 *        application pointer.

 * 
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

/**
 * @brief Implements the singleton design pattern to access a global 
 *        application pointer.
 */
class Application: public QVTKApplication
{
public:
    Application(int& c, char** v);
    virtual bool notify(QObject *rec, QEvent *ev);    

public:
    static boost::shared_ptr<Application> Get (int& c, char** v);
    static boost::shared_ptr<Application> Get ();
    static void release ();

private:
    static boost::shared_ptr<Application> m_application;
};



#endif //__APPLICATION_H__

// Local Variables:
// mode: c++
// End:
