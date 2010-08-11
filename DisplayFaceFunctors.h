/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Functors that display a face
 */

#ifndef __DISPLAY_FACE_FUNCTORS_H__
#define __DISPLAY_FACE_FUNCTORS_H__

#include "DisplayEdgeFunctors.h"

/**
 * Functor that displays a face
 */
template <typename displayEdges>
class DisplayFace : public DisplayElementProperty
{
public:
    /**
     * Constructor
     * @param widget Where should be the face displayed
     */
    DisplayFace (const GLWidget& widget, FocusContext focus = FOCUS, 
		 BodyProperty::Enum bodyProperty = BodyProperty::NONE) : 
	DisplayElementProperty (widget, focus, bodyProperty), m_count(0) 
    {
    }
    /**
     * Functor that displays a face
     * @param f the face to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedFace>& of)
    {
        if (m_widget.IsDisplayedFace (m_count))
        {
	    display (of);
	    if (m_count == m_widget.GetDisplayedFaceIndex ())
		cdbg << "face " << m_count << ": " << *of << endl;
        }
        m_count++;
    }

    void operator () (const boost::shared_ptr<Face>& f)
    {
	boost::shared_ptr<OrientedFace> of = 
	    boost::make_shared<OrientedFace>(f, false);
	operator() (of);
    }


protected:
    virtual void display (const boost::shared_ptr<OrientedFace>& of)
    {
	if (m_focus == FOCUS)
	{
	    glColor (G3D::Color4 (Color::BLACK, 1.));
	}
	else
	    glColor (G3D::Color4 (Color::BLACK, m_widget.GetContextAlpha ()));
	(displayEdges (m_widget, m_focus)) (of);
    }

private:
    /**
     * Used to display fewer faces (for DEBUG purposes)
     */
    size_t m_count;
};


/**
 * Functor that displays a face using the color specified in the DMP file
 */
class DisplayFaceWithColor : public DisplayFace<DisplaySameEdges>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    DisplayFaceWithColor (
	const GLWidget& widget,
	FocusContext focus = FOCUS,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE) : 
	DisplayFace<DisplaySameEdges> (widget, focus, bodyProperty) 
    {
    }

protected:
    virtual void display (const boost::shared_ptr<OrientedFace>& of)
    {
	if (m_focus == FOCUS)
	{
	    if (m_bodyProperty == BodyProperty::NONE)
	    {
		Color::Enum color = of->GetColor ();
		glColor (Color::GetValue(color));
	    }
	    else
	    {
		size_t bodyId = of->GetBodyPartOf ().GetBodyId ();
		QColor color;
		const FoamAlongTime& foamAlongTime = 
		    m_widget.GetFoamAlongTime ();
		if (foamAlongTime.ExistsBodyProperty (
			m_bodyProperty, bodyId, m_widget.GetTimeStep ()))
		{
		    float value = foamAlongTime.GetBodyProperty (
			m_bodyProperty, bodyId, m_widget.GetTimeStep ());
		    color = m_widget.MapScalar (value);
		}
		else
		    color = m_widget.GetNotAvailableFaceColor ();
		m_widget.qglColor (color);
	    }
	}
	else
	    glColor (G3D::Color4 (Color::GetValue(Color::BLACK),
				  m_widget.GetContextAlpha ()));
	(DisplaySameEdges (m_widget)) (of);
    }
};

/**
 * Displays a face and specifies the normal to the face. Used for lighting.
 */
class DisplayFaceWithNormal : public DisplayFace<DisplaySameEdges>
{
public:
    /**
     * Constructor
     * @param widget where to display the face
     */
    DisplayFaceWithNormal (
	const GLWidget& widget, 
	FocusContext focus = FOCUS,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE) : 
	DisplayFace<DisplaySameEdges> (widget, focus, bodyProperty)
    {
    }

protected:
    /**
     * Functor used to display a face together to the normal
     * @param f face to be displayed
     */
    virtual void display (const boost::shared_ptr<OrientedFace>& f)
    {
	using G3D::Vector3;
	Vector3 normal = f->GetNormal ();
	glNormal (normal);

	// specify the vertices
	(DisplaySameEdges (m_widget)) (f);
    }
};


#endif //__DISPLAY_FACE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
