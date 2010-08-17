/**
 * @file RestrictedRangeSlider.cpp
 * @author Dan R. Lipsa
 * @date 16 Aug. 2010
 *
 * Method implementation for RestrictedRangeSlider object
 */

#include "RestrictedRangeSlider.h"

RestrictedRangeSlider::RestrictedRangeSlider (QWidget *parent) :
    QSlider (parent),
    m_originalMin (minimum ()),
    m_originalMax (maximum ()),
    m_state (FULL_RANGE)
{
    connect (this, SIGNAL(valueChanged (int)),
	     this, SLOT(ToOriginalRange (int)));
}

void RestrictedRangeSlider::SetRestrictedTo (
    const vector<bool>& selectedIntervals)
{
    if (GetState () == FULL_RANGE)
    {
	m_originalMin = minimum ();
	m_originalMax = maximum ();
    }
    if (count (
	    selectedIntervals.begin (), selectedIntervals.end (), false) == 0)
    {
	SetFullRange ();
	return;
    }
    m_toOriginalRange.reserve (selectedIntervals.size ());
    m_toOriginalRange.resize (0);
    for (size_t i = 0; i < selectedIntervals.size (); ++i)
	if (selectedIntervals[i])
	    m_toOriginalRange.push_back (i);
    if (m_toOriginalRange.size () == 0)
    {
	m_state = EMPTY_RANGE;
	setMinimum (0);
	setMaximum (0);
    }
    else
    {
	m_state = RESTRICTED_RANGE;
	setMinimum (0);
	setMaximum (m_toOriginalRange.size () - 1);
    }
    setValue (0);
    ToOriginalRange (0);
}

void RestrictedRangeSlider::SetFullRange ()
{
    m_state = FULL_RANGE;
    setMinimum (m_originalMin);
    setMaximum (m_originalMax);
    m_toOriginalRange.resize (0);
    setValue (m_originalMin);
    ToOriginalRange (m_originalMin);
}

void RestrictedRangeSlider::ToOriginalRange (int value)
{
    switch (GetState ())
    {
    case FULL_RANGE:
	Q_EMIT valueChangedOriginalRange (value);
	break;
    case RESTRICTED_RANGE:
	Q_EMIT valueChangedOriginalRange (m_toOriginalRange [value]);
	break;
    case EMPTY_RANGE:
	Q_EMIT valueChangedNone ();
	break;
    }
}
