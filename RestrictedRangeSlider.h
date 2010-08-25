/**
 * @file RestrictedRangeSlider.h
 * @author Dan R. Lipsa
 *
 * Declaration of RestrictedRangeSlider object
 */
#ifndef __RESTRICTED_RANGE_SLIDER_H__
#define __RESTRICTED_RANGE_SLIDER_H__

class RestrictedRangeSlider : public QGroupBox
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
	m_slider.setMinimum (min);
	setupScale (minimum (), maximum ());
    }
    void setMaximum (int max)
    {
	m_slider.setMaximum (max);
	setupScale (minimum (), maximum ());
    }
    void setSingleStep (int singleStep)
    {
	m_slider.setSingleStep (singleStep);
    }
    void setPageStep (int pageStep)
    {
	m_slider.setPageStep (pageStep);
    }
    int value () const
    {
	return m_slider.value ();
    }
    void setValue (int value)
    {
	m_slider.setValue (value);
    }
    int minimum () const
    {
	return m_slider.minimum ();
    }
    int maximum () const
    {
	return m_slider.maximum ();
    }


public Q_SLOTS:
    void ToOriginalRange (int value);

Q_SIGNALS:
    void valueChangedOriginalRange (int value);
    void valueChangedNone ();

private:
    void setupUi ();
    void updateTitle ();
    void setupScale (int min, int max);
    void setupColorMap (const vector<bool>* selected = 0);
    QColor toColor (bool selected)
    {
	return selected ? Qt::black : Qt::white;
    }

private:
    Q_OBJECT
    QwtScaleWidget m_scale;
    QSlider m_slider;
    vector<int> m_toOriginalRange;
    State m_state;
    boost::shared_ptr<QwtLinearColorMap> m_colorMap;
};


#endif //__RESTRICTED_RANGE_SLIDER_H__

// Local Variables:
// mode: c++
// End:
