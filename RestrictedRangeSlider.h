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
    int minimum () const
    {
	return slider->minimum ();
    }
    int maximum () const
    {
	return slider->maximum ();
    }


public Q_SLOTS:
    void ValueChangedSlider (int value);

Q_SIGNALS:
    void valueChanged (int value);

private:
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
};


#endif //__RESTRICTED_RANGE_SLIDER_H__

// Local Variables:
// mode: c++
// End:
