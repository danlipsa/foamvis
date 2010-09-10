/**
 * @file   ColorBarModel.h
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Declaration of the ColorBarModel class
 */

#ifndef __COLOR_BAR_MODEL_H__
#define __COLOR_BAR_MODEL_H__

#include "Enums.h"

class ColorBarModel
{
public:
    ColorBarModel ();
    const QwtDoubleInterval& GetInterval () const
    {
	return m_interval;
    }
    const QwtLinearColorMap& GetColorMap () const
    {
	return m_colorMap;
    }

    void SetInterval (QwtDoubleInterval interval)
    {
	m_interval = interval;
	m_clampValues = interval;
    }
    void SetClampHigh (double clampHigh)
    {
	m_clampValues.setMaxValue (clampHigh);
    }
    void SetClampLow (double clampLow)
    {
	m_clampValues.setMinValue (clampLow);
    }
    void SetClampValues (const QwtDoubleInterval& clampValues)
    {
	m_clampValues = clampValues;
    }
    void SetClampClear ()
    {
	m_clampValues = m_interval;
    }

    void SetTitle (const char* title)
    {
	m_title = title;
    }
    const QString& GetTitle () const
    {
	return m_title;
    }
    
    void SetupPalette (Palette::Enum palette);
    Palette::Enum GetPalette () const
    {
	return m_palette;
    }

    QColor MapScalar (double value) const
    {
	return m_colorMap.color (m_interval, value);
    }

private:
    void setupPaletteRainbowTelea ();
    void setupPaletteBlackBody ();
    void setupPaletteRainbowHSV ();
    void setupPaletteDiverging (size_t c);

    template<typename ColorMapper>
    void setupColorMap (ColorMapper colorMapper);


private:
    static const size_t COLORS;

private:
    Palette::Enum m_palette;
    QwtLinearColorMap m_colorMap;
    QwtDoubleInterval m_interval;
    QwtDoubleInterval m_clampValues;
    QString m_title;
};


#endif //__COLOR_BAR_MODEL_H__

// Local Variables:
// mode: c++
// End:
