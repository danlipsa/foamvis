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
class TexCoordSetter
{
public:
    TexCoordSetter (const GLWidget& glWidget) :
	m_glWidget (glWidget)
    {
    }

    void operator () (double value);
    void operator () ();

private:
    const GLWidget& m_glWidget;
};

class VertexAttributeSetter
{
public:
    VertexAttributeSetter (QGLShaderProgram& program, int attributeIndex, 
			   const GLWidget& glWidget) :
	m_program (program), m_attributeIndex (attributeIndex), 
	m_glWidget(glWidget)
    {
    }

    VertexAttributeSetter (const GLWidget& glWidget) :
	m_program (*(QGLShaderProgram*)0), m_attributeIndex (0),
	m_glWidget (glWidget)
    {
	(void)glWidget;
    }

    void operator () (double value)
    {
	m_program.setAttributeValue (m_attributeIndex, value);
    }
    void operator () ();

private:
    QGLShaderProgram& m_program;
    int m_attributeIndex;
    const GLWidget& m_glWidget;
};

#endif //__PROPERTY_SETTER_H__

// Local Variables:
// mode: c++
// End:
