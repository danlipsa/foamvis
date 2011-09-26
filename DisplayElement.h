/**
 * @file   DisplayElement.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display an element (body, face, edge or vertex)
 */

#ifndef __DISPLAY_ELEMENT_H__
#define __DISPLAY_ELEMENT_H__

#include "Enums.h"
#include "PropertySetter.h"
class GLWidget;
class FoamProperties;

class DisplayElement
{
public:
    enum FocusContext
    {
	FOCUS,
	CONTEXT
    };
    enum ContextType
    {
	USER_DEFINED_CONTEXT,
	INVISIBLE_CONTEXT
    };

    enum TessellationEdgesDisplay
    {
	TEST_DISPLAY_TESSELLATION,
	DONT_DISPLAY_TESSELLATION
    };

public:
    DisplayElement (
	const GLWidget& widget, const FoamProperties& foamProperties,
	bool useZPos = false, double zPos = 0) : 
	m_glWidget (widget), m_foamProperties (foamProperties),
	m_useZPos (useZPos),
	m_zPos (zPos)
    {
    }

protected:
    const GLWidget& m_glWidget;
    const FoamProperties& m_foamProperties;
    bool m_useZPos;
    double m_zPos;
};


class DisplayElementFocus : public DisplayElement
{
public:
    DisplayElementFocus (
	const GLWidget& widget, const FoamProperties& fp,
	FocusContext focus = FOCUS, bool useZPos = false, double zPos = 0) :
	
	DisplayElement (widget, fp, useZPos, zPos),
	m_focus (focus)
    {
    }
    
protected:
    FocusContext m_focus;
};

class SetterTextureCoordinate;

template<typename PropertySetter = SetterTextureCoordinate>
class DisplayElementProperty : public DisplayElement
{
public:
    DisplayElementProperty (
	const GLWidget& glWidget, const FoamProperties& fp, 
	PropertySetter propertySetter,
	bool useZPos = false, double zPos = 0) :

	DisplayElement (glWidget, fp, useZPos, zPos), 
	m_propertySetter (propertySetter)
    {
    }
protected:
    PropertySetter m_propertySetter;
};



template<typename PropertySetter = SetterTextureCoordinate>
class DisplayElementPropertyFocus : 
    public DisplayElementProperty<PropertySetter>
{
public:
    DisplayElementPropertyFocus (
	const GLWidget& widget, const FoamProperties& fp,
	PropertySetter setter,
	DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, double zPos = 0) :
	
	DisplayElementProperty<PropertySetter> (
	    widget, fp, setter, useZPos, zPos),
	m_focus (focus)
    {
    }
protected:
    DisplayElement::FocusContext m_focus;
};


#endif //__DISPLAY_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
