/**
 * @file RestrictedRangeSlider.cpp
 * @author Dan R. Lipsa
 * @date 16 Aug. 2010
 *
 * Method implementation for RestrictedRangeSlider object
 */

#include "RestrictedRangeSlider.h"

RestrictedRangeSlider::RestrictedRangeSlider (QWidget *parent) :
    QGroupBox (parent),
    m_scale (QwtScaleDraw::TopScale, this),
    m_slider (Qt::Horizontal, this),
    m_state (FULL_RANGE)
{
    setupUi ();
    connect (&m_slider, SIGNAL(valueChanged (int)),
	     this, SLOT(ToOriginalRange (int)));
}

void RestrictedRangeSlider::setupUi ()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget (&m_scale);
    layout->addWidget (&m_slider);
    setLayout (layout);
}

void RestrictedRangeSlider::setupScale (int minimum, int maximum)
{
    const QwtDoubleInterval interval (minimum, maximum);
    QwtLinearScaleEngine scaleEngine;
    QwtScaleDiv scaleDiv;
    const int maxMajorTicks = 8;
    const int maxMinorTicks = 5;

    scaleDiv = scaleEngine.divideScale (
	interval.minValue (), interval.maxValue (), 
	maxMajorTicks, maxMinorTicks);
    m_scale.setScaleDiv (scaleEngine.transformation (), scaleDiv);
    setupColorMap ();
    m_scale.setBorderDist (15, 15);
}

void RestrictedRangeSlider::setupColorMap (const vector<bool>* selected)
{
    vector<bool> all;
    const vector<bool>* v;
    const QwtDoubleInterval interval (minimum (), maximum ());
    m_colorMap = boost::make_shared<QwtLinearColorMap> ();
    if (selected == 0)
    {
	v = &all;
	all.resize (maximum () - minimum () + 1, true);
    }
    else
	v = selected;
    m_colorMap->setColorInterval (toColor ((*v)[0]),
				 toColor ((*v)[(*v).size () - 1]));
    m_colorMap->setMode (QwtLinearColorMap::FixedColors);
    for (size_t i = 1; i < (*v).size () - 1; ++i)
    {
	double value = static_cast<double>(i) / (*v).size ();
	m_colorMap->addColorStop (value, toColor ((*v)[i]));
    }
    m_scale.setColorMap (interval, *m_colorMap);
    m_scale.setColorBarEnabled (true);
}



void RestrictedRangeSlider::SetRestrictedTo (
    const vector<bool>& selectedIntervals)
{
    setupColorMap (&selectedIntervals);
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
    }
    else
    {
	m_state = RESTRICTED_RANGE;
    }
    updateTitle ();
}

void RestrictedRangeSlider::SetFullRange ()
{
    m_state = FULL_RANGE;
    m_toOriginalRange.resize (0);
    updateTitle ();
}

void RestrictedRangeSlider::updateTitle ()
{
    if (GetState () != RestrictedRangeSlider::FULL_RANGE)
    {
	ostringstream ostr;
	size_t range = 
	    (GetState () == RestrictedRangeSlider::EMPTY_RANGE) ? 
	    0 : m_toOriginalRange.size ();
	ostr << "Selected Time Steps: " 
	     << range << " of " << (maximum () - minimum () + 1);
	setTitle (ostr.str ().c_str ());
    }
    else
	setTitle ("Time Steps");    
}

void RestrictedRangeSlider::ToOriginalRange (int value)
{
    Q_EMIT valueChangedOriginalRange (value);
}
