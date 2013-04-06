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
    const QwtLinearColorMap& GetQwtColorMap () const
    {
	return m_qwtColorMap;
    }
    vtkSmartPointer<vtkColorTransferFunction> GetVtkColorMap () const
    {
	return m_vtkColorMap;
    }
    const QImage& GetImage () const
    {
	return m_image;
    }    
    const string& GetTitle () const
    {
	return m_title;
    }
    void SetTitle (const char* title)
    {
	m_title = title;
    }

    Palette GetPalette () const
    {
	return m_palette;
    }
    void SetInterval (QwtDoubleInterval interval)
    {
	m_interval = interval;
	m_clampInterval = interval;
    }
    const QwtDoubleInterval GetInterval () const
    {
	return m_interval;
    }
    void SetClampInterval (const QwtDoubleInterval& clampValues)
    {
	m_clampInterval = clampValues;
        SetupPalette (GetPalette ());
    }
    const QwtDoubleInterval& GetClampInterval () const
    {
	return m_clampInterval;
    }
    void SetClampClear ()
    {
	m_clampInterval = m_interval;
	SetupPalette (GetPalette ());
    }
    void SetClampMax (double clampHigh)
    {
	m_clampInterval.setMaxValue (clampHigh);
        SetupPalette (GetPalette ());
    }
    double GetClampMax () const
    {
        return m_clampInterval.maxValue ();
    }
    void SetClampMin (double clampLow)
    {
	m_clampInterval.setMinValue (clampLow);
        SetupPalette (GetPalette ());
    }
    double GetClampMin () const
    {
        return m_clampInterval.minValue ();
    }
    void SetClampMaxMinimum ()
    {
	m_clampInterval.setMinValue (m_interval.minValue ());
	m_clampInterval.setMaxValue (m_interval.minValue ());
	SetupPalette (GetPalette ());
    }    
    void SetupPalette (Palette palette);
    QColor GetHighlightColor (HighlightNumber::Enum i) const;
    void SetHighlightColor (HighlightNumber::Enum i, const QColor& color)
    {
	m_highlightColor[i] = color;
    }
    double TexCoord (double value) const;
    void SetLog10 (bool log10);
    bool IsLog10 () const
    {
        return m_log10;
    }
    bool IsClampedMin () const;
    bool IsClampedMax () const;
    float GetClampMinRatio () const;
    float GetClampMaxRatio () const;
    string ToString () const;
    void ColorMapCopy (const ColorBarModel& other);
    G3D::Vector2 GetBarLabelSize () const;


    static const size_t COLORS;

private:    
    void setupPaletteRainbowHSV ();
    void setupPaletteRainbow ();
    void setupPaletteRainbowExtended ();
    void setupPaletteDiverging (size_t c);
    void setupPaletteDiverging (PaletteDiverging::Enum paletteDiverging);

    void setupPaletteSequential (PaletteSequential::Enum paletteSequential);
    void setupPaletteSequentialBlackBody ();
    void setupPaletteSequentialBrewerBlues9 ();
    void setupPaletteSequentialBrewerYlOrRd9 ();

    void setup ();
    void setupColorMap ();
    void setupImage ();
    QColor getColor (double value) const;
    void adjustColorTransferFunction ();

private:
    Palette m_palette;
    QwtLinearColorMap m_qwtColorMap;
    QImage m_image;
    QwtDoubleInterval m_interval;
    QwtDoubleInterval m_clampInterval;
    // maps [0, 1] to a range of colors
    vtkSmartPointer<vtkColorTransferFunction> m_ctf;
    vtkSmartPointer<vtkColorTransferFunction> m_vtkColorMap;
    string m_title;
    boost::array<QColor,HighlightNumber::COUNT> m_highlightColor;
    bool m_log10;
};

inline ostream& operator<< (ostream& ostr, const ColorBarModel& b)
{
    return ostr << b.ToString ();
}


#endif //__MODEL_H__

// Local Variables:
// mode: c++
// End:
