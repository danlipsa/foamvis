/**
 * @file   PropertySetter.h
 * @author Dan R. Lipsa
 * @date  30 October 2010
 *
 * Sends a property value to the graphics card
 */

#ifndef __PROPERTY_SETTER_H__
#define __PROPERTY_SETTER_H__

class GLWidget;
class Body;
class SetterTextureCoordinate
{
public:
    SetterTextureCoordinate (
	const GLWidget& glWidget, ViewNumber::Enum view) :

	m_glWidget (glWidget), m_viewNumber (view)
    {
    }

    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
    int GetBodyOrFaceProperty () const;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }

protected:
    const GLWidget& m_glWidget;
    ViewNumber::Enum m_viewNumber;
};

class SetterVertexAttribute : public SetterTextureCoordinate
{
public:
    SetterVertexAttribute (
	const GLWidget& glWidget,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterTextureCoordinate (glWidget, view),
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
	const GLWidget& glWidget,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (glWidget, view, program, attributeIndex)
    {
    }
    int GetBodyOrFaceProperty () const;
    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
};

class SetterVelocity : public SetterVertexAttribute
{
public:
    SetterVelocity (
	const GLWidget& glWidget,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (glWidget, view, program, attributeIndex)
    {
    }

    int GetBodyOrFaceProperty () const;
    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
};



class SetterNop : public SetterVertexAttribute
{
public:
    SetterNop (
	const GLWidget& glWidget,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (glWidget, view, program, attributeIndex)
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
