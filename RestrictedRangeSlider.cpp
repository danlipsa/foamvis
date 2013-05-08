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
    updateLabelTime (minimum ());
    updateLabelSelected ();
    updateLabelTimeSteps ();
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
    updateLabelTimeSteps ();
}

void RestrictedRangeSlider::setupColorMap (const vector<bool>* selected)
{
    vector<bool> v;
    const size_t WIDTH = 3;
    const QwtDoubleInterval interval (minimum (), maximum ());
    m_qwtColorMap = boost::make_shared<QwtLinearColorMap> ();
    if (selected == 0)
    {
	v.resize (maximum () - minimum () + 1, true);
    }
    else
	ensureMinimumWidth (*selected, &v, getMinimumWidth (selected->size ()));
    m_qwtColorMap->setColorInterval (toColor (v[0]),
				  toColor (v[v.size () - 1]));
    m_qwtColorMap->setMode (QwtLinearColorMap::ScaledColors);
    size_t colors = v.size () - 1;
    for (size_t i = 1; i < colors; ++i)
    {
	double value = static_cast<double>(i) / colors;
	QColor color = toColor (v[i]);
	m_qwtColorMap->addColorStop (value, color);
    }
    bool restrictedRange = (GetState () != RestrictedRangeSlider::FULL_RANGE);
    scale->setColorBarEnabled (restrictedRange);
    scale->setColorMap (interval, *m_qwtColorMap);
    scale->setColorBarWidth (WIDTH);
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
    size_t unselectedIntervalCount = count (
	selectedIntervals.begin (), selectedIntervals.end (), false);
    if (unselectedIntervalCount == 0)
	SetFullRange ();
    else
    {
        m_toOriginalRange.reserve (selectedIntervals.size ());
        m_toOriginalRange.resize (0);
        for (size_t i = 0; i < selectedIntervals.size (); ++i)
            if (selectedIntervals[i])
                m_toOriginalRange.push_back (i);
        if (m_toOriginalRange.empty ())
        {
            m_state = EMPTY_RANGE;
        }
        else
        {
            m_state = RESTRICTED_RANGE;
        }
    }
    setupColorMap (&selectedIntervals);
    updateLabelSelected ();
}

void RestrictedRangeSlider::SetFullRange ()
{
    m_state = FULL_RANGE;
    m_toOriginalRange.resize (0);
    updateLabelSelected ();
    scale->setColorBarEnabled (false);
}

void RestrictedRangeSlider::updateLabelSelected ()
{
    ostringstream ostr;
    bool restrictedRange = (GetState () != RestrictedRangeSlider::FULL_RANGE);
    labelSelectedTitle->setShown (restrictedRange);
    labelSelected->setShown (restrictedRange);
    if (restrictedRange)
    {
	size_t range = 
	    (GetState () == RestrictedRangeSlider::EMPTY_RANGE) ? 
	    0 : m_toOriginalRange.size ();
	ostr << range;
        labelSelected->setText (ostr.str ().c_str ());
    }
}

void RestrictedRangeSlider::updateLabelTimeSteps ()
{
    ostringstream ostr;
    ostr << (maximum () - minimum () + 1);
    labelTimeSteps->setText (ostr.str ().c_str ());
}

void RestrictedRangeSlider::updateLabelTime (int value)
{
    ostringstream ostr;
    ostr << value;
    labelTime->setText (ostr.str ().c_str ());
}


void RestrictedRangeSlider::ValueChangedSlider (int value)
{
    updateLabelTime (value);
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

void RestrictedRangeSlider::SetValueAndMaxNoSignals (
    size_t value, size_t maximum)
{
    blockSignals (true);
    setMaximum (maximum);
    setValue (value);
    blockSignals (false);
}


void RestrictedRangeSlider::SetValueNoSignals (size_t value)
{
    blockSignals (true);
    setValue (value);
    blockSignals (false);
}
