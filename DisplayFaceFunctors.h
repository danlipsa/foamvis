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
#include "DisplayEdgeFunctors.h"
#include "Enums.h"

class Face;
class OrientedFace;
class DisplayFaceLineStrip;

/**
 * Functor that displays a face using a color map highlight color
 */
template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, 
	  typename PropertySetter = SetterValueTextureCoordinate>
class DisplayFaceHighlightColor : 
    public DisplayElementPropertyFocus<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayFaceHighlightColor (
	const GLWidget& widget,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0,
	bool useZPos = false,
	double zPos = 0);
    
    DisplayFaceHighlightColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false,
	double zPos = 0);

    /**
     * Functor that displays a face
     */
    void operator() (const boost::shared_ptr<OrientedFace>& of);
    
    void operator () (const boost::shared_ptr<Face>& f);
};


/**
 * Functor that displays a face using the color specified in the DMP file or
 * a color mapped body property value
 */
template<typename PropertySetter = SetterValueTextureCoordinate>
class DisplayFaceBodyPropertyColor : 
    public DisplayFaceHighlightColor<HighlightNumber::H0, 
				     DisplayFaceTriangleFan, PropertySetter>
{
public:
    DisplayFaceBodyPropertyColor (
	const GLWidget& widget,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceBodyPropertyColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, 
	double zPos = 0);

    void operator () (const boost::shared_ptr<OrientedFace>& of);

private:
    void setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
			    bool* useColor);
};


template<QRgb faceColor,
	 typename PropertySetter = SetterValueTextureCoordinate>
class DisplayFaceLineStripColor : 
    public DisplayFaceHighlightColor<HighlightNumber::H0, 
				     DisplayFaceLineStrip, PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    DisplayFaceLineStripColor (
	const GLWidget& widget,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceLineStripColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, 
	double zPos = 0);
    void operator () (const boost::shared_ptr<OrientedFace>& of);
    void operator () (const boost::shared_ptr<Face>& f);
};



template<QRgb faceColor,
	 typename PropertySetter = SetterValueTextureCoordinate>
class DisplayFaceDmpColor : 
    public DisplayFaceHighlightColor<HighlightNumber::H0, 
				     DisplayFaceTriangleFan, PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    DisplayFaceDmpColor (
	const GLWidget& widget,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceDmpColor (
	const GLWidget& widget,
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, 
	double zPos = 0);

    void operator () (const boost::shared_ptr<OrientedFace>& of);
    void operator () (const boost::shared_ptr<Face>& f);
private:
    void displayNoNormal (const boost::shared_ptr<Face>& f);
};




#endif //__DISPLAY_FACE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
