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
    PaletteType::Enum GetPaletteType () const
    {
	return m_paletteType;
    }
    PaletteSequential::Enum GetPaletteSequential () const
    {
	return m_paletteSequential;
    }
    PaletteDiverging::Enum GetPaletteDiverging () const
    {
	return m_paletteDivergent;
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
	SetupPalette (GetPaletteType (), GetPaletteSequential (),
		      GetPaletteDiverging ());
    }

    void SetTitle (const char* title)
    {
	m_title = title;
    }
    
    void SetupPalette (PaletteType::Enum type, 
		       PaletteSequential::Enum paletteSequential,
		       PaletteDiverging::Enum paletteDivergent);
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
    PaletteType::Enum m_paletteType;
    PaletteSequential::Enum m_paletteSequential;
    PaletteDiverging::Enum m_paletteDivergent;
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
