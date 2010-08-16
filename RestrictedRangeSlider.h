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
    RestrictedRangeSlider (QWidget *parent);
    void SetRestrictedTo (const vector< pair<size_t, size_t> >& intervals);
    void SetUnrestricted ();
    bool IsRestricted ();

public Q_SLOTS:
    void ToOriginalRange (int value);

Q_SIGNALS:
    void valueChangedOriginalRange (int value);

private:
    Q_OBJECT
    vector<size_t> m_toOriginalRange;
    size_t m_originalMin;
    size_t m_originalMax;
};


#endif //__RESTRICTED_RANGE_SLIDER_H__

// Local Variables:
// mode: c++
// End:
