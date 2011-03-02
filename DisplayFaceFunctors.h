/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Functors that display a face
 */

#ifndef __DISPLAY_FACE_FUNCTORS_H__
#define __DISPLAY_FACE_FUNCTORS_H__

#include "DisplayElement.h"

class Face;
class OrientedFace;
class DisplayFaceLineStrip;

/**
 * Functor that displays a face
 */
template <size_t highlightColorIndex,
	  typename displayEdges, 
	  typename PropertySetter = SetterValueTextureCoordinate>
class DisplayFaceWithHighlightColor : 
    public DisplayElementPropertyFocus<PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget Where should be the face displayed
     */
    DisplayFaceWithHighlightColor (const GLWidget& widget,
		 typename DisplayElement::FocusContext focus = 
		 DisplayElement::FOCUS,
		 BodyProperty::Enum property = BodyProperty::NONE,
		 bool useZPos = false, double zPos = 0);
    
    DisplayFaceWithHighlightColor (const GLWidget& widget,
		 PropertySetter propertySetter,
		 typename DisplayElement::FocusContext focus = 
		 DisplayElement::FOCUS,
		 bool useZPos = false, double zPos = 0);

    /**
     * Functor that displays a face
     * @param f the face to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedFace>& of);

    void operator () (const boost::shared_ptr<Face>& f);

protected:
    virtual void display (const boost::shared_ptr<OrientedFace>& of);

private:
    /**
     * Used to display fewer faces (for DEBUG purposes)
     */
    size_t m_count;
};


/**
 * Functor that displays a face using the color specified in the DMP file
 */
template<typename displaySameEdges = DisplayFaceLineStrip, 
	 typename PropertySetter = SetterValueTextureCoordinate>
class DisplayFaceWithBodyPropertyColor : 
    public DisplayFaceWithHighlightColor<0, displaySameEdges, PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    DisplayFaceWithBodyPropertyColor (
	const GLWidget& widget,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	BodyProperty::Enum property = BodyProperty::NONE, 
	bool useZPos = false, double zPos = 0);

    DisplayFaceWithBodyPropertyColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, double zPos = 0);

protected:
    virtual void display (const boost::shared_ptr<OrientedFace>& of);

private:
    void setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
			    bool* useColor);
};

#endif //__DISPLAY_FACE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
