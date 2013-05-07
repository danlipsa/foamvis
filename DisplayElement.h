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
class Settings;
class ViewSettings;
class Foam;

class DisplayElement
{
public:
    enum FocusContext
    {
	FOCUS,
	CONTEXT
    };

    enum TessellationEdgesDisplay
    {
	DISPLAY_TESSELLATION_EDGES,
	DONT_DISPLAY_TESSELLATION_EDGES
    };

public:
    DisplayElement (
	const Settings& settings, bool useZPos = false, double zPos = 0) : 
	m_settings (settings),
	m_useZPos (useZPos),
	m_zPos (zPos)
    {
    }

protected:
    const Settings& m_settings;
    bool m_useZPos;
    double m_zPos;
};


class DisplayElementFocus : public DisplayElement
{
public:
    DisplayElementFocus (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
	FocusContext focus = FOCUS, bool useZPos = false, double zPos = 0) :
	
	DisplayElement (settings, useZPos, zPos),
        m_viewNumber (viewNumber),
        m_is2D (is2D),
	m_focus (focus)
    {
    }
    ViewNumber::Enum GetViewNumber () const
    {
        return m_viewNumber;
    }
    bool Is2D () const
    {
        return m_is2D;
    }
    const ViewSettings& GetViewSettings () const;

protected:
    ViewNumber::Enum m_viewNumber;
    const bool m_is2D;
    FocusContext m_focus;
};

class SetterTextureCoordinate;

template<typename PropertySetter = SetterTextureCoordinate>
class DisplayElementProperty : public DisplayElement
{
public:
    DisplayElementProperty (
	const Settings& settings, 
	PropertySetter propertySetter,
	bool useZPos = false, double zPos = 0) :

	DisplayElement (settings, useZPos, zPos), 
	m_propertySetter (propertySetter)
    {
    }
    ViewNumber::Enum GetViewNumber () const;
    bool Is2D () const;
    const ViewSettings& GetViewSettings () const;


protected:
    PropertySetter m_propertySetter;
};



template<typename PropertySetter = SetterTextureCoordinate>
class DisplayElementPropertyFocus : 
    public DisplayElementProperty<PropertySetter>
{
public:
    DisplayElementPropertyFocus (
	const Settings& settings, 
	PropertySetter setter,
	DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, double zPos = 0) :
	
	DisplayElementProperty<PropertySetter> (
	    settings, setter, useZPos, zPos),
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
