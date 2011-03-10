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
class SetterValueTextureCoordinate
{
public:
    SetterValueTextureCoordinate (const GLWidget& glWidget, ViewNumber::Enum view) :
	m_glWidget (glWidget), m_viewNumber (view)
    {
    }

    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);
    BodyProperty::Enum GetBodyProperty () const;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }

protected:
    const GLWidget& m_glWidget;
    ViewNumber::Enum m_viewNumber;
};

class SetterValueVertexAttribute : public SetterValueTextureCoordinate
{
public:
    SetterValueVertexAttribute (
	const GLWidget& glWidget,  ViewNumber::Enum view,
	QGLShaderProgram* program = 0, int attributeIndex = 0) :
	SetterValueTextureCoordinate (glWidget, view),
	m_program (program), m_attributeIndex (attributeIndex)
    {
    }

    void operator () ();
    void operator () (const boost::shared_ptr<Body>& body);

private:
    QGLShaderProgram* m_program;
    int m_attributeIndex;
};


#endif //__PROPERTY_SETTER_H__

// Local Variables:
// mode: c++
// End:
