/**
 * @file RestrictedRangeSlider.h
 * @author Dan R. Lipsa
 *
 * Declaration of RestrictedRangeSlider object
 */
#ifndef __RESTRICTED_RANGE_SLIDER_H__
#define __RESTRICTED_RANGE_SLIDER_H__

class RestrictedRangeSlider : public QSlider
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
    int GetOriginalMinimum () const
    {
	return m_originalMin;
    }
    int GetOriginalMaximum () const
    {
	return m_originalMax;
    }

public Q_SLOTS:
    void ToOriginalRange (int value);

Q_SIGNALS:
    void valueChangedOriginalRange (int value);
    void valueChangedNone ();

private:
    Q_OBJECT
    vector<int> m_toOriginalRange;
    int m_originalMin;
    int m_originalMax;
    State m_state;
};


#endif //__RESTRICTED_RANGE_SLIDER_H__

// Local Variables:
// mode: c++
// End:
