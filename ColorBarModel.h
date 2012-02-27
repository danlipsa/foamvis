/**
 * @file   ColorBarModel.h
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Declaration of the ColorBarModel class
 */

#ifndef __MODEL_H__
#define __MODEL_H__

#include "Enums.h"

class ColorBarModel
{
public:
    ColorBarModel ();
    const QwtLinearColorMap& GetColorMap () const
    {
	return m_colorMap;
    }
    const QImage& GetImage () const
    {
	return m_image;
    }    
    const QwtDoubleInterval& GetClampValues () const
    {
	return m_clampValues;
    }
    const QwtDoubleInterval& GetInterval () const
    {
	return m_interval;
    }
    const QString& GetTitle () const
    {
	return m_title;
    }
    Palette GetPalette () const
    {
	return m_palette;
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
	SetupPalette (GetPalette ());
    }
    void SetClampHighMinimum ()
    {
	m_clampValues.setMinValue (m_interval.minValue ());
	m_clampValues.setMaxValue (m_interval.minValue ());
	SetupPalette (GetPalette ());
    }

    void SetTitle (const char* title)
    {
	m_title = title;
    }
    
    void SetupPalette (Palette palette);
    QColor GetHighlightColor (HighlightNumber::Enum i) const;
    void SetHighlightColor (HighlightNumber::Enum i, const QColor& color)
    {
	m_highlightColor[i] = color;
    }
    double TexCoord (double value) const;
    string ToString () const;

    static const size_t COLORS;

private:
    void setupPaletteRainbowTelea ();
    void setupPaletteRainbowHSV ();
    void setupPaletteRainbow ();
    void setupPaletteRainbowExtended ();
    void setupPaletteBlackBody ();
    void setupPaletteBrewerBlue9 ();
    void setupPaletteDiverging (size_t c);
    void setupPaletteDiverging (PaletteDiverging::Enum paletteDiverging);
    void setupPaletteSequential (PaletteSequential::Enum paletteSequential);

    template<typename ColorMapper>
    void setup (ColorMapper colorMapper);
    template<typename ColorMapper>
    void setupColorMap (ColorMapper colorMapper);
    template<typename ColorMapper>
    void setupImage (ColorMapper colorMapper);

private:
    Palette m_palette;
    QwtLinearColorMap m_colorMap;
    QImage m_image;
    QwtDoubleInterval m_interval;
    QwtDoubleInterval m_clampValues;
    QString m_title;
    boost::array<QColor,HighlightNumber::COUNT> m_highlightColor;
};

inline ostream& operator<< (ostream& ostr, const ColorBarModel& b)
{
    return ostr << b.ToString ();
}


#endif //__MODEL_H__

// Local Variables:
// mode: c++
// End:
