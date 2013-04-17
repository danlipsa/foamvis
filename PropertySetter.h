/**
 * @file   PropertySetter.h
 * @author Dan R. Lipsa
 * @date  30 October 2010
 *
 * Sends a property value to the graphics card
 */

#ifndef __PROPERTY_SETTER_H__
#define __PROPERTY_SETTER_H__

class Settings;
class Body;
class SetterTextureCoordinate
{
public:
    SetterTextureCoordinate (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D) :

	m_settings (settings), m_viewNumber (viewNumber), m_is2D (is2D)
    {
    }

    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
    int GetBodyOrOtherScalar () const;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
    bool Is2D () const
    {
        return m_is2D;
    }

protected:
    G3D::Matrix3 getRotation () const;

protected:
    const Settings& m_settings;
    ViewNumber::Enum m_viewNumber;
    const bool m_is2D;
};

class SetterVertexAttribute : public SetterTextureCoordinate
{
public:
    SetterVertexAttribute (
	const Settings& settings,  ViewNumber::Enum viewNumber, bool is2D,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :

	SetterTextureCoordinate (settings, viewNumber, is2D),
	m_program (program), m_attributeLocation (attributeIndex)
    {
    }

    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);

protected:
    QGLShaderProgram* m_program;
    int m_attributeLocation;
};

class SetterDeformation : public SetterVertexAttribute
{
public:
    SetterDeformation (
	const Settings& settings,  ViewNumber::Enum viewNumber, bool is2D,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
        
	SetterVertexAttribute (settings, viewNumber, is2D,
                               program, attributeIndex)
    {
    }
    int GetBodyOrOtherScalar () const;
    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
};

class SetterVelocity : public SetterVertexAttribute
{
public:
    SetterVelocity (
	const Settings& settings,  ViewNumber::Enum viewNumber, bool is2D,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :

	SetterVertexAttribute (settings, viewNumber, is2D,
                               program, attributeIndex)
    {
    }

    int GetBodyOrOtherScalar () const;
    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
};



class SetterNop : public SetterVertexAttribute
{
public:
    SetterNop (
	const Settings& settings,  ViewNumber::Enum viewNumber, bool is2D,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (settings, viewNumber, is2D, 
                               program, attributeIndex)
    {
    }

    inline void operator () ()
    {
    }
    
    inline void operator () (const boost::shared_ptr<Body>& body)
    {
	(void)body;
    }
};




#endif //__PROPERTY_SETTER_H__

// Local Variables:
// mode: c++
// End:
