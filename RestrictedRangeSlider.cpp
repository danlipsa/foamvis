/**
 * @file RestrictedRangeSlider.cpp
 * @author Dan R. Lipsa
 * @date 16 Aug. 2010
 *
 * Method implementation for RestrictedRangeSlider object
 */

#include "RestrictedRangeSlider.h"

RestrictedRangeSlider::RestrictedRangeSlider (QWidget *parent) :
    QSlider (parent)
{
    connect (this, SIGNAL(valueChanged (int)),
	     this, SLOT(ToOriginalRange (int)));
}

void RestrictedRangeSlider::SetRestrictedTo (
    const vector< pair<size_t, size_t> >& selectedIntervals)
{
    pair<size_t, size_t> interval;
    BOOST_FOREACH (interval, selectedIntervals)
	for (size_t i = interval.first; i < interval.second; i++)
	    m_toOriginalRange.push_back (i);
    m_originalMin = minimum ();
    m_originalMax = maximum ();
    setMinimum (0);
    setMaximum (m_toOriginalRange.size () - 1);
    if (value () == 0)
	Q_EMIT ToOriginalRange (0);
    setValue (0);
}

void RestrictedRangeSlider::SetUnrestricted ()
{
    setMinimum (m_originalMin);
    setMaximum (m_originalMax);
    m_toOriginalRange.resize (0);
}

bool RestrictedRangeSlider::IsRestricted ()
{
    return m_toOriginalRange.size () != 0;
}

void RestrictedRangeSlider::ToOriginalRange (int value)
{
    int valueOriginalRange = 
	IsRestricted () ? m_toOriginalRange [value] : value;
    Q_EMIT valueChangedOriginalRange (valueOriginalRange);
}
