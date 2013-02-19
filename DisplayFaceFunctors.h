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
 * Functor that displays an edge
 */
class DisplayFaceTriangleFan : public DisplayElementFocus
{
public:
    DisplayFaceTriangleFan (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
	FocusContext focus = FOCUS,
	bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (settings, viewNumber, is2D, focus, useZPos, zPos)
    {
    }

    void operator () (const boost::shared_ptr<Face>& f) const;
    void operator () (const boost::shared_ptr<const OrientedFace>& of) const
    {
	operator () (of.get ());
    }
    void operator () (const OrientedFace* of) const;
};


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
	const Settings& settings, bool is2D,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum viewNumber = ViewNumber::VIEW0,
	bool useZPos = false,
	double zPos = 0);
    
    DisplayFaceHighlightColor (
	const Settings& settings, 
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
class DisplayFaceBodyScalarColor : 
    public DisplayFaceHighlightColor<HighlightNumber::H0, 
				     DisplayFaceTriangleFan, PropertySetter>
{
public:
    DisplayFaceBodyScalarColor (
	const Settings& settings, bool is2D,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceBodyScalarColor (
	const Settings& settings, 
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
    DisplayFaceDmpColor (
	const Settings& settings, 
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	ViewNumber::Enum view = ViewNumber::VIEW0, 
	bool useZPos = false,
	double zPos = 0);

    DisplayFaceDmpColor (
	const Settings& settings, 
	PropertySetter propertySetter,
	typename DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, 
	double zPos = 0);

    void operator () (const boost::shared_ptr<OrientedFace>& of);
    void operator () (const boost::shared_ptr<Face>& f);
private:
    void displayNoNormal (const boost::shared_ptr<Face>& f);
};

// Display all edges of a face
// ======================================================================
class DisplayFaceLineStrip : public DisplayElementFocus
{
public:
    DisplayFaceLineStrip (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
	FocusContext focus = FOCUS,
	bool useZPos = false, double zPos = 0) :

	DisplayElementFocus (settings, viewNumber, is2D, focus, useZPos, zPos)
    {
    }
    void operator() (const boost::shared_ptr<OrientedFace>& of);

    void operator() (const boost::shared_ptr<Face>& f);
};




template<typename displayEdge>
class DisplayFaceEdges : public DisplayElementFocus
{
public:
    DisplayFaceEdges (
        const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
        FocusContext focus,                       
        bool useZPos = false, double zPos = 0);

    void operator() (const boost::shared_ptr<OrientedFace> f);

    void operator () (const boost::shared_ptr<Face> f);
};



#endif //__DISPLAY_FACE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
