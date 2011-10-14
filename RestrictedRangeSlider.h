/**
 * @file RestrictedRangeSlider.h
 * @author Dan R. Lipsa
 *
 * Declaration of RestrictedRangeSlider object
 */
#ifndef __RESTRICTED_RANGE_SLIDER_H__
#define __RESTRICTED_RANGE_SLIDER_H__

#include "ui_RestrictedRangeSlider.h"

class RestrictedRangeSlider : public QWidget, 
			      private Ui::RestrictedRangeSlider
{
public:
    enum State
    {
	FULL_RANGE,
	RESTRICTED_RANGE,
	EMPTY_RANGE
    };

public:
    RestrictedRangeSlider (QWidget *parent);
    void SetRestrictedTo (const vector<bool>& intervals);
    void SetFullRange ();
    State GetState () const
    {
	return m_state;
    }

    void setMinimum (int min)
    {
	slider->setMinimum (min);
	setupScale (minimum (), maximum ());
    }
    void setMaximum (int max)
    {
	slider->setMaximum (max);
	setupScale (minimum (), maximum ());
    }
    void setSingleStep (int singleStep)
    {
	slider->setSingleStep (singleStep);
    }
    void setPageStep (int pageStep)
    {
	slider->setPageStep (pageStep);
    }
    int value () const
    {
	return slider->value ();
    }
    void setValue (int value)
    {
	slider->setValue (value);
	updateLabelValue (value);
    }
    void SetValueAndMaxNoSignals (size_t value, size_t maximum);
    int minimum () const
    {
	return slider->minimum ();
    }
    int maximum () const
    {
	return slider->maximum ();
    }
    void SetTitle (const string& title)
    {
	m_title = title;
    }
    string GetTitle () const
    {
	return m_title;
    }
    void NextSelected ();
    void PreviousSelected ();

public Q_SLOTS:
    void ValueChangedSlider (int value);

Q_SIGNALS:
    void valueChanged (int value);

private:
    /**
     * Ensures that for one index selected in the source we have at least
     * 2*w + 1 elements selected in the destination. Used for the color bar,
     * otherwise for many elements just one element selected won't be visible.
     */
    void ensureMinimumWidth (
	const vector<bool> source, vector<bool>* destination, size_t w);
    /**
     * Returns the minimum width of an selected element for size elements in the
     * range.
     */
    size_t getMinimumWidth (size_t size);
    void updateTitle ();
    void updateLabelValue (int value);
    void setupScale (int min, int max);
    void setupColorMap (const vector<bool>* selected = 0);
    QColor toColor (bool selected)
    {
	return selected ? Qt::black : Qt::lightGray;
    }

private:
    Q_OBJECT

    vector<int> m_toOriginalRange;
    State m_state;
    boost::shared_ptr<QwtLinearColorMap> m_colorMap;
    string m_title;
};


#endif //__RESTRICTED_RANGE_SLIDER_H__

// Local Variables:
// mode: c++
// End:
