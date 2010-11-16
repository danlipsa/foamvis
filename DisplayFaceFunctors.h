/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Functors that display a face
 */

#ifndef __DISPLAY_FACE_FUNCTORS_H__
#define __DISPLAY_FACE_FUNCTORS_H__

#include "FoamAlongTime.h"
#include "DisplayEdgeFunctors.h"

/**
 * Functor that displays a face
 */
template <typename displayEdges, typename PropertySetter = TexCoordSetter>
class DisplayFace : public DisplayElementPropertyFocus<PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget Where should be the face displayed
     */
    DisplayFace (const GLWidget& widget,
		 typename DisplayElement::FocusContext focus = 
		 DisplayElement::FOCUS,
		 BodyProperty::Enum bodyProperty = BodyProperty::NONE,
		 bool useZPos = false, double zPos = 0) : 

	DisplayElementPropertyFocus<PropertySetter> (
	    widget, PropertySetter (widget), bodyProperty, focus, 
	    useZPos, zPos), 
	m_count(0)
    {
    }



    DisplayFace (const GLWidget& widget,
		 PropertySetter propertySetter,
		 typename DisplayElement::FocusContext focus = 
		 DisplayElement::FOCUS,
		 BodyProperty::Enum bodyProperty = BodyProperty::NONE, 
		 bool useZPos = false, double zPos = 0) : 
	DisplayElementPropertyFocus<PropertySetter> (
	    widget, propertySetter, bodyProperty, focus, useZPos, zPos), 
	m_count(0)
    {
    }

    /**
     * Functor that displays a face
     * @param f the face to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedFace>& of)
    {
        if (this->m_glWidget.IsDisplayedFace (m_count))
        {
	    display (of);
	    if (m_count == this->m_glWidget.GetDisplayedFaceIndex ())
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
	if (this->m_focus == DisplayElement::FOCUS)
	{
	    glColor (G3D::Color4 (Color::BLACK, 1.));
	}
	else
	    glColor (
		G3D::Color4 (Color::BLACK, this->m_glWidget.GetContextAlpha ()));
	(displayEdges (this->m_glWidget, this->m_focus, 
		       this->m_useZPos, this->m_zPos)) (of);
    }

private:
    /**
     * Used to display fewer faces (for DEBUG purposes)
     */
    size_t m_count;
};


/**
 * Functor that displays a face using the color specified in the DMP file
 * @todo Fix the back front facing for faces.
 */
template<typename displaySameEdges = DisplaySameEdges, 
	 typename PropertySetter = TexCoordSetter>
class DisplayFaceWithColor : 
    public DisplayFace<displaySameEdges, PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    DisplayFaceWithColor (
	const GLWidget& widget,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE, 
	bool useZPos = false, double zPos = 0) : 

	DisplayFace<displaySameEdges, PropertySetter> (
	    widget, PropertySetter (widget), focus, bodyProperty, useZPos, zPos)
    {
    }


    DisplayFaceWithColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE, 
	bool useZPos = false, double zPos = 0) : 

	DisplayFace<displaySameEdges, PropertySetter> (
	    widget, propertySetter, focus, bodyProperty, useZPos, zPos) 
    {}



protected:
    virtual void display (const boost::shared_ptr<OrientedFace>& of)
    {
	/*
	  glColor (Qt::black);
	  G3D::Vector3 first = *of->GetOrientedEdge (0).GetBegin ();
	  DisplayOrientedEdge displayOrientedEdge;
	  displayOrientedEdge(first, first + normal);
	*/

	glNormal (of->GetNormal ());
	bool useColor = true;
	if (this->m_focus == DisplayElement::FOCUS)
	{
	    if (this->m_bodyProperty == BodyProperty::NONE)
		glColor (Color::GetValue(of->GetColor ()));
	    else
	    {
		size_t bodyId = of->GetBodyPartOf ().GetBodyId ();
		QColor color;
		const FoamAlongTime& foamAlongTime = 
		    this->m_glWidget.GetFoamAlongTime ();
		if (foamAlongTime.ExistsBodyProperty (
			this->m_bodyProperty, bodyId, 
			this->m_glWidget.GetTimeStep ()))
		{
		    double value = foamAlongTime.GetBodyProperty (
			this->m_bodyProperty, bodyId, 
			this->m_glWidget.GetTimeStep ());
		    this->m_propertySetter (value);
		    useColor = false;
		}
		else
		    glColor (this->m_glWidget.GetNotAvailableFaceColor ());
	    }
	}
	else
	    glColor (G3D::Color4 (Color::GetValue(Color::BLACK),
				  this->m_glWidget.GetContextAlpha ()));
	if (useColor)
	    glDisable (GL_TEXTURE_1D);
	(displaySameEdges (this->m_glWidget)) (of);
	if (useColor)
	    glEnable (GL_TEXTURE_1D);
    }
};

#endif //__DISPLAY_FACE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
