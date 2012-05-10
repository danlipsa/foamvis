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
	  typename PropertySetter = SetterTextureCoordinate>
class DisplayFaceHighlightColor : 
    public DisplayElementPropertyFocus<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayFaceHighlightColor (
	const GLWidget& widget, const FoamProperties& fp,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0,
	bool useZPos = false,
	double zPos = 0);
    
    DisplayFaceHighlightColor (
	const GLWidget& widget, const FoamProperties& fp,
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
 * @todo: color by the number of edges of a face in 3D
 */
template<typename PropertySetter = SetterTextureCoordinate>
class DisplayFaceBodyPropertyColor : 
    public DisplayFaceHighlightColor<HighlightNumber::H0, 
				     DisplayFaceTriangleFan, PropertySetter>
{
public:
    DisplayFaceBodyPropertyColor (
	const GLWidget& widget, const FoamProperties& fp,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceBodyPropertyColor (
	const GLWidget& widget, const FoamProperties& fp,
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
	 typename PropertySetter = SetterTextureCoordinate>
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
	const GLWidget& widget, const FoamProperties& fp,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceDmpColor (
	const GLWidget& widget, const FoamProperties& fp,
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
