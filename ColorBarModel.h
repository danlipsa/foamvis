/**
 * @file   ColorBarModel.h
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Declaration of the ColorBarModel class
 */

#ifndef __COLOR_BAR_MODEL_H__
#define __COLOR_BAR_MODEL_H__

class ColorBarModel
{
public:
    void SetInterval (QwtDoubleInterval interval)
    {
	m_interval = interval;
    }
    const QwtDoubleInterval& GetInterval () const
    {
	return m_interval;
    }
    const QwtLinearColorMap& GetColorMap () const
    {
	return m_colorMap;
    }

    void SetupRainbowColorMap ();
    void SetupBlueRedColorMap ();

    QColor MapScalar (float value) const
    {
	return m_colorMap.color (m_interval, value);
    }

private:
    QwtLinearColorMap m_colorMap;
    QwtDoubleInterval m_interval;    
};


#endif //__COLOR_BAR_MODEL_H__

// Local Variables:
// mode: c++
// End:
