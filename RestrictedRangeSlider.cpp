/**
 * @file RestrictedRangeSlider.cpp
 * @author Dan R. Lipsa
 * @date 16 Aug. 2010
 *
 * Method implementation for RestrictedRangeSlider object
 */

#include "RestrictedRangeSlider.h"
#include "Application.h"

// Private Classes
// ======================================================================

class SliderQuery : public QSlider
{
public:
    SliderQuery (QApplication* app) :
	QSlider (Qt::Horizontal), m_app (app)
    {
    }

    QRect GetHandleRect ()
    {
	QStyle* s = m_app->style ();
	QStyleOptionSlider styleOptionSlider;
	initStyleOption (&styleOptionSlider);
	return s->subControlRect (
	    QStyle::CC_Slider, &styleOptionSlider, QStyle::SC_SliderHandle,
	    this);
    }

private:
    QApplication* m_app;
};

// Methods
// ======================================================================

RestrictedRangeSlider::RestrictedRangeSlider (QWidget *parent) :
    QWidget (parent),
    m_state (FULL_RANGE)
{
    setupUi (this);
    scale->setAlignment (QwtScaleDraw::TopScale);
    updateTitle ();
    updateLabelValue (minimum ());
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
    scale->setScaleDiv (scaleEngine.transformation (), scaleDiv);
    setupColorMap ();
    SliderQuery sliderQuery (Application::Get ().get ());
    QRect r = sliderQuery.GetHandleRect ();
    scale->setBorderDist (r.width () / 2, r.width () / 2);
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
	double value = static_cast<double>(i) / ((*v).size () - 1);
	m_colorMap->addColorStop (value, toColor ((*v)[i]));
    }
    scale->setColorMap (interval, *m_colorMap);
    scale->setColorBarEnabled (true);
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
    ostringstream ostr;
    if (GetState () != RestrictedRangeSlider::FULL_RANGE)
    {
	size_t range = 
	    (GetState () == RestrictedRangeSlider::EMPTY_RANGE) ? 
	    0 : m_toOriginalRange.size ();
	ostr << "Selected Time Steps: " 
	     << range << " of " << (maximum () - minimum () + 1);
    }
    else
	ostr << "Time Steps: " << (maximum () - minimum () + 1);
    groupBoxRestrictedRange->setTitle (ostr.str ().c_str ());
}

void RestrictedRangeSlider::updateLabelValue (int value)
{
    ostringstream ostr;
    ostr << value;
    labelValue->setText (ostr.str ().c_str ());
}


void RestrictedRangeSlider::ValueChangedSlider (int value)
{
    updateLabelValue (value);
    Q_EMIT valueChanged (value);
}
