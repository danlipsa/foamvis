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
#include "DisplayElement.h"

class Face;
class OrientedFace;

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
		 BodyProperty::Enum property = BodyProperty::NONE,
		 bool useZPos = false, double zPos = 0);

    DisplayFace (const GLWidget& widget,
		 PropertySetter propertySetter,
		 typename DisplayElement::FocusContext focus = 
		 DisplayElement::FOCUS,
		 BodyProperty::Enum property = BodyProperty::NONE, 
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
	BodyProperty::Enum property = BodyProperty::NONE, 
	bool useZPos = false, double zPos = 0);

    DisplayFaceWithColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	BodyProperty::Enum property = BodyProperty::NONE, 
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
