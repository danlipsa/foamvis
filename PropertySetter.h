/**
 * @file   PropertySetter.h
 * @author Dan R. Lipsa
 * @date  30 October 2010
 *
 * Sends a property value to the graphics card
 */

#ifndef __PROPERTY_SETTER_H__
#define __PROPERTY_SETTER_H__

class WidgetGl;
class Body;
class SetterTextureCoordinate
{
public:
    SetterTextureCoordinate (
	const WidgetGl& widgetGl, ViewNumber::Enum view) :

	m_widgetGl (widgetGl), m_viewNumber (view)
    {
    }

    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
    int GetFaceScalar () const;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }

protected:
    G3D::Matrix3 getRotation () const;

protected:
    const WidgetGl& m_widgetGl;
    ViewNumber::Enum m_viewNumber;
};

class SetterVertexAttribute : public SetterTextureCoordinate
{
public:
    SetterVertexAttribute (
	const WidgetGl& widgetGl,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterTextureCoordinate (widgetGl, view),
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
	const WidgetGl& widgetGl,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (widgetGl, view, program, attributeIndex)
    {
    }
    int GetFaceScalar () const;
    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
};

class SetterVelocity : public SetterVertexAttribute
{
public:
    SetterVelocity (
	const WidgetGl& widgetGl,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (widgetGl, view, program, attributeIndex)
    {
    }

    int GetFaceScalar () const;
    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
};



class SetterNop : public SetterVertexAttribute
{
public:
    SetterNop (
	const WidgetGl& widgetGl,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterVertexAttribute (widgetGl, view, program, attributeIndex)
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
