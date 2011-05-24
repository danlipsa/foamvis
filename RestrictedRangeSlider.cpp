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

class SliderDimensionsQuery : public QSlider
{
public:
    SliderDimensionsQuery (QApplication* app) :
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

    SliderDimensionsQuery sliderQuery (Application::Get ().get ());
    QRect r = sliderQuery.GetHandleRect ();
    scale->setBorderDist (r.width () / 2, r.width () / 2);

    updateTitle ();
}

void RestrictedRangeSlider::setupColorMap (const vector<bool>* selected)
{
    vector<bool> v;
    const size_t COLOR_BAR_WIDTH = 3;
    const QwtDoubleInterval interval (minimum (), maximum ());
    m_colorMap = boost::make_shared<QwtLinearColorMap> ();
    if (selected == 0)
    {
	v.resize (maximum () - minimum () + 1, true);
    }
    else
	ensureMinimumWidth (*selected, &v, getMinimumWidth (selected->size ()));
    m_colorMap->setColorInterval (toColor (v[0]),
				  toColor (v[v.size () - 1]));
    m_colorMap->setMode (QwtLinearColorMap::ScaledColors);
    size_t colors = v.size () - 1;
    for (size_t i = 1; i < colors; ++i)
    {
	double value = static_cast<double>(i) / colors;
	QColor color = toColor (v[i]);
	m_colorMap->addColorStop (value, color);
    }
    scale->setColorMap (interval, *m_colorMap);
    scale->setColorBarEnabled (false);
    scale->setColorBarWidth (COLOR_BAR_WIDTH);
}

void RestrictedRangeSlider::ensureMinimumWidth (
    const vector<bool> source, vector<bool>* destination, size_t w)
{
    vector<bool>& v = *destination;
    v.resize (source.size (), false);
    for (size_t i = 0; i < v.size (); ++i)
    {
	if (source[i])
	{
	    for (size_t j = 0; j < w; ++j)
	    {
		if (i >= j)
		    v[i-j] = true;
		if ((i+j) < v.size ())
		    v[i+j] = true;
	    }
	}
    }
}

size_t RestrictedRangeSlider::getMinimumWidth (size_t size)
{
    if (size < 100)
	return 1;
    else if (size < 300)
	return 2;
    else if (size < 500)
	return 3;
    else
	return 4;
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
    scale->setColorBarEnabled (true);
    updateTitle ();
}

void RestrictedRangeSlider::SetFullRange ()
{
    m_state = FULL_RANGE;
    m_toOriginalRange.resize (0);
    updateTitle ();
    scale->setColorBarEnabled (false);
}

void RestrictedRangeSlider::updateTitle ()
{
    ostringstream ostr;
    if (GetState () != RestrictedRangeSlider::FULL_RANGE)
    {
	size_t range = 
	    (GetState () == RestrictedRangeSlider::EMPTY_RANGE) ? 
	    0 : m_toOriginalRange.size ();
	ostr << string ("Selected ") + ": "
	     << range << " of " << (maximum () - minimum () + 1);
    }
    else
	ostr << GetTitle () << ": " << (maximum () - minimum () + 1);
    labelInfo->setText (ostr.str ().c_str ());
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

void RestrictedRangeSlider::NextSelected ()
{
    if (m_state == RESTRICTED_RANGE)
    {
	vector<int>::iterator it = upper_bound (
	    m_toOriginalRange.begin (), m_toOriginalRange.end (), value ());
	if (it != m_toOriginalRange.end ())
	    setValue (*it);
    }
    else
	slider->triggerAction (QSlider::SliderSingleStepAdd);
}

void RestrictedRangeSlider::PreviousSelected ()
{
    if (m_state == RESTRICTED_RANGE)
    {
	vector<int>::iterator it = lower_bound (
	    m_toOriginalRange.begin (), m_toOriginalRange.end (), value ());
	if (it != m_toOriginalRange.begin ())
	    setValue (*(it - 1));
	
    }
    else
	slider->triggerAction (QSlider::SliderSingleStepSub);
}
