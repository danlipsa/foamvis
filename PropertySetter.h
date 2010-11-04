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

private:
    const GLWidget& m_glWidget;
};

class VertexAttributeSetter
{
public:
    VertexAttributeSetter (QGLShaderProgram& program, int attributeIndex) :
	m_program (program), m_attributeIndex (attributeIndex)
    {
    }

    void operator () (double value)
    {
	m_program.setAttributeValue (m_attributeIndex, value);
    }

private:
    QGLShaderProgram& m_program;
    int m_attributeIndex;
};

#endif //__PROPERTY_SETTER_H__

// Local Variables:
// mode: c++
// End:
