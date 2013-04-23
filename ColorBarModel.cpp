/**
 * @file   ColorBarModel.cpp
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Definition of the ColorBarModel class
 */

#include "ColorBarModel.h"
#include "Debug.h"
#include "Settings.h"
#include "Utils.h"

// Private functions and classes
// ======================================================================

// Methods
// ======================================================================

const size_t ColorBarModel::COLORS = 256;

ColorBarModel::ColorBarModel () :
    m_image (COLORS, 1, QImage::Format_RGB32),
    m_interval (0, 1),
    m_clampInterval (0, 1),
    m_log10 (false)
{
    m_ctf = vtkColorTransferFunction::New ();
    m_vtkColorMap = vtkColorTransferFunction::New ();
}

QColor ColorBarModel::GetHighlightColor (HighlightNumber::Enum i) const
{
    return m_highlightColor[i];
}

QColor ColorBarModel::GetColor (double value) const
{
    QColor color;
    color.setRedF (m_ctf->GetRedValue (value));
    color.setGreenF (m_ctf->GetGreenValue (value));
    color.setBlueF (m_ctf->GetBlueValue (value));
    return color;    
}

void ColorBarModel::setupPaletteRainbow ()
{
    m_ctf->RemoveAllPoints ();
    m_ctf->SetColorSpaceToLab();
    m_ctf->AddRGBPoint(0.0, 0, 0, 1);   // blue
    m_ctf->AddRGBPoint(0.25, 0, 1, 1);   // cyan
    m_ctf->AddRGBPoint(0.50, 0, 1, 0);   // green
    m_ctf->AddRGBPoint(0.75, 1, 1, 0);   // yellow
    m_ctf->AddRGBPoint(1.0, 1, 0, 0);   // red
    setup ();
    m_highlightColor[0] = Qt::darkBlue;
    m_highlightColor[1] = Qt::yellow;
    m_highlightColor[2] = Qt::white;
}

void ColorBarModel::setupPaletteRainbowExtended ()
{
    m_ctf->RemoveAllPoints ();
    m_ctf->SetColorSpaceToLab();
    m_ctf->AddRGBPoint(0.0, 1, 0, 1);   // magenta
    m_ctf->AddRGBPoint(0.2, 0, 0, 1);   // blue
    m_ctf->AddRGBPoint(0.4, 0, 1, 1);   // cyan
    m_ctf->AddRGBPoint(0.6, 0, 1, 0);   // green
    m_ctf->AddRGBPoint(0.8, 1, 1, 0);   // yellow
    m_ctf->AddRGBPoint(1.0, 1, 0, 0);   // red
    setup ();    
    m_highlightColor[0] = Qt::darkBlue;
    m_highlightColor[1] = Qt::yellow;
    m_highlightColor[2] = Qt::white;
}


void ColorBarModel::SetupPalette (Palette palette)
{
    m_palette.m_type = palette.m_type;
    m_palette.m_sequential = PaletteSequential::BLACK_BODY;
    m_palette.m_diverging = PaletteDiverging::BLUE_RED;
    if (palette.m_type == PaletteType::SEQUENTIAL)
	setupPaletteSequential (palette.m_sequential);
    else if (palette.m_type == PaletteType::DIVERGING)
	setupPaletteDiverging (palette.m_diverging);
}

void ColorBarModel::setupPaletteSequential (
    PaletteSequential::Enum paletteSequential)
{
    m_palette.m_sequential = paletteSequential;
    switch (paletteSequential)
    {
    case PaletteSequential::BLACK_BODY:
	setupPaletteSequentialBlackBody ();
	break;
    case PaletteSequential::BREWER_BLUES9:
	setupPaletteSequentialBrewerBlues9 ();
	break;
    case PaletteSequential::BREWER_YLORRD9:
	setupPaletteSequentialBrewerYlOrRd9 ();
	break;
    case PaletteSequential::COUNT:
	ThrowException ("Invalid sequential palette: ", paletteSequential);
    }
}

void ColorBarModel::setupPaletteDiverging (
    PaletteDiverging::Enum paletteDiverging)
{
    m_palette.m_diverging = paletteDiverging;
    switch (paletteDiverging)
    {
    case PaletteDiverging::BLUE_RED:
    case PaletteDiverging::BLUE_TAN:
    case PaletteDiverging::PURPLE_ORANGE:
    case PaletteDiverging::GREEN_PURPLE:
    case PaletteDiverging::GREEN_RED:
	setupPaletteDiverging (paletteDiverging - PaletteDiverging::BLUE_RED);
	break;
    case PaletteDiverging::COUNT:
	ThrowException ("Invalid diverging palette: ", paletteDiverging);
    }
}

void ColorBarModel::setupPaletteSequentialBlackBody ()
{
  m_ctf->RemoveAllPoints ();
  m_ctf->SetColorSpaceToLab();
  m_ctf->AddRGBPoint(0.0, 0, 0, 0);   // black
  m_ctf->AddRGBPoint(0.33, 1, 0, 0);   // red
  m_ctf->AddRGBPoint(0.66, 1, 1, 0);   // yellow
  m_ctf->AddRGBPoint(1.0, 1, 1, 1);   // white
  setup ();
  m_highlightColor[0] = Qt::green;
  m_highlightColor[1] = Qt::blue;
  m_highlightColor[2] = Qt::magenta;
}

void ColorBarModel::setupPaletteSequentialBrewerBlues9 ()
{
    m_ctf->RemoveAllPoints ();
    m_ctf->SetColorSpaceToLab();
    //m_ctf->AddRGBPoint(0.0    , 0.968627, 0.984314, 1.000000);
    m_ctf->AddRGBPoint(0.0    ,        1,        1, 1.000000); // change to white
    m_ctf->AddRGBPoint(0.12500, 0.870588, 0.921569, 0.968627);
    m_ctf->AddRGBPoint(0.25000, 0.776471, 0.858824, 0.937255);
    m_ctf->AddRGBPoint(0.37500, 0.619608, 0.792157, 0.882353);
    m_ctf->AddRGBPoint(0.50000, 0.419608, 0.682353, 0.839216);
    m_ctf->AddRGBPoint(0.62500, 0.258824, 0.572549, 0.776471);
    m_ctf->AddRGBPoint(0.75000, 0.129412, 0.443137, 0.709804);
    m_ctf->AddRGBPoint(0.87500, 0.031373, 0.317647, 0.611765);
    m_ctf->AddRGBPoint(1.00000, 0.031373, 0.188235, 0.419608);
    setup ();
    m_highlightColor[0] = Qt::darkBlue;
    m_highlightColor[1] = Qt::red;
    m_highlightColor[2] = Qt::green;
}

void ColorBarModel::setupPaletteSequentialBrewerYlOrRd9 ()
{
    m_ctf->RemoveAllPoints ();
    m_ctf->SetColorSpaceToLab();
    m_ctf->AddRGBPoint(0.0    , 1.00000, 1.00000, 0.80000);
    m_ctf->AddRGBPoint(0.12500, 1.00000, 0.92941, 0.62745);
    m_ctf->AddRGBPoint(0.25000, 0.99608, 0.85098, 0.46275);
    m_ctf->AddRGBPoint(0.37500, 0.99608, 0.69804, 0.29804);
    m_ctf->AddRGBPoint(0.50000, 0.99216, 0.55294, 0.23529);
    m_ctf->AddRGBPoint(0.62500, 0.98824, 0.30588, 0.16471);
    m_ctf->AddRGBPoint(0.75000, 0.89020, 0.10196, 0.10980);
    m_ctf->AddRGBPoint(0.87500, 0.74118, 0.00000, 0.14902);
    m_ctf->AddRGBPoint(1.00000, 0.50196, 0.00000, 0.14902);
    setup ();
    m_highlightColor[0] = Qt::darkBlue;
    m_highlightColor[1] = Qt::blue;
    m_highlightColor[2] = Qt::green;
}




void ColorBarModel::setupPaletteRainbowHSV ()
{
  m_ctf->RemoveAllPoints ();
  m_ctf->SetColorSpaceToHSV();
  m_ctf->HSVWrapOff();
  m_ctf->AddHSVPoint(0.0, 0.66667, 1.0, 1.0); // blue
  m_ctf->AddHSVPoint(1.0, 0.0, 1.0, 1.0);     // red
  setup ();
}


/**
 * Kenneth Moreland. "Diverging Color Maps for Scientific Visualization." 
 * In Proceedings of the 5th International Symposium on Visual Computing, 
 * December 2009.
 * http://www.cs.unm.edu/~kmorel/documents/ColorMaps/index.html
 */
void ColorBarModel::setupPaletteDiverging (size_t c)
{
    // two divergent color + 3 highlight colors
    const static double colors[][5][3] = 
	{
	    /* blue red*/
	    {{0.230, 0.299, 0.754},
	     {0.706, 0.016, 0.150},
	     {0, 0, 0.5}, {0, 1, 1}, {0, 1, 0} // darkBlue, cyan, green
	    },

	    /* blue to tan */
	    {{0.217, 0.525, 0.910},
	     {0.677, 0.492, 0.093},
	     {0, 0, 0.5}, {1, 0, 0}, {0, 1, 0}
	    },

	    /* purple to orange */
	    {{0.436, 0.308, 0.631},
	     {0.759, 0.334, 0.046},
	     {0, 0, 0.5}, {0, 1, 0}, {0, 0, 1}
	    },

	    /* green to purple */
	    {{0.085, 0.532, 0.201},
	     {0.436, 0.308, 0.631},
	     {0, 0, 0.5}, {1, 1, 0}, {1, 0, 0}
	    },

	    /* green to red */
	    {{0.085, 0.532, 0.201},
	     {0.758, 0.214, 0.233},
	     {0, 0, 0.5}, {1, 1, 0}, {0, 0, 1}
	    }
	};    
    m_ctf->RemoveAllPoints ();
    m_ctf->SetColorSpaceToDiverging();
    m_ctf->AddRGBPoint(
	0.0, colors[c][0][0], colors[c][0][1], colors[c][0][2]);
    m_ctf->AddRGBPoint(
	1.0, colors[c][1][0], colors[c][1][1], colors[c][1][2]);
    setup ();
    for (size_t i = 0; i < m_highlightColor.size (); ++i)
	m_highlightColor[i] = QColor::fromRgbF (
	    colors[c][i+2][0], colors[c][i+2][1], colors[c][i+2][2]);
}

void ColorBarModel::setup ()
{
    setupColorMap ();
    setupImage ();
    adjustColorTransferFunction ();
}

void ColorBarModel::adjustColorTransferFunction ()
{
    // [0,1] -> m_clampInterval 
    m_vtkColorMap->RemoveAllPoints ();
    m_vtkColorMap->SetColorSpace (m_ctf->GetColorSpace ());
    double leftColor[3], rightColor[3];
    m_ctf->GetColor (0, leftColor);
    m_vtkColorMap->AddRGBPoint (
        m_interval.minValue (), leftColor[0], leftColor[1], leftColor[2]);
    m_ctf->GetColor (1, rightColor);
    m_vtkColorMap->AddRGBPoint (
        m_interval.maxValue (), rightColor[0], rightColor[1], rightColor[2]);
    for (int i = 0; i < m_ctf->GetSize (); ++i)
    {
	double v[6];
	m_ctf->GetNodeValue (i, v);
	v[0] = m_clampInterval.minValue () +
	    v[0] * (m_clampInterval.maxValue () - m_clampInterval.minValue ());
	m_vtkColorMap->AddRGBPoint (v[0], v[1], v[2], v[3]);
    }
}


void ColorBarModel::setupColorMap ()
{
    m_qwtColorMap.setColorInterval (GetColor (0), GetColor (1));
    double width = m_interval.width ();
    double low = 
        (m_clampInterval.minValue () -  m_interval.minValue ()) / width;
    double high = 
        (m_clampInterval.maxValue () -  m_interval.minValue ()) / width;
    if (low != 0)
	m_qwtColorMap.addColorStop (low, GetColor (0));
    if (high != 1)
	m_qwtColorMap.addColorStop (high, GetColor (1));
    m_qwtColorMap.setMode (QwtLinearColorMap::FixedColors);
    size_t colors = COLORS - 1;
    for (size_t i = 1; i < colors; ++i)
    {
	double value = static_cast<double>(i) / colors;
	double insideClamp = low + value * (high - low);
	m_qwtColorMap.addColorStop (insideClamp, GetColor (value));
    }
}


void ColorBarModel::setupImage ()
{
    double width = m_interval.width ();
    double low = (m_clampInterval.minValue () -  m_interval.minValue ()) / width;
    double high = 
        (m_clampInterval.maxValue () -  m_interval.minValue ()) / width;
    size_t colors = COLORS - 1;
    for (size_t i = 0; i <= colors; i++)
    {
	double value = static_cast<double>(i) / colors;
	uint rgb;
	if (value <= low)
	    rgb = static_cast<uint> (
		GetColor (0).rgb ());
	else if (value >= high)
	    rgb = static_cast<uint> (
		GetColor (1).rgb ());
	else
	{
	    double inside01 = (value - low) / (high - low);
	    rgb = static_cast<uint> (
		GetColor (inside01).rgb ());
	}
	m_image.setPixel (i, 0, rgb);
    }
}

double ColorBarModel::TexCoord (double value) const
{
    RuntimeAssert (m_interval.contains (value),
		   "Value: ", value, " outside interval: ", m_interval);
    return (value - m_interval.minValue ()) / 
	(m_interval.maxValue () - m_interval.minValue ());
}

string ColorBarModel::ToString () const
{
    ostringstream ostr;
    ostr << "palette: " << m_palette
	 << ", interval: " << m_interval
	 << ", clamping: " << m_clampInterval << endl;
    return ostr.str ();
}

void ColorBarModel::SetLog10 (bool log10)
{
    m_log10 = log10;
}

bool ColorBarModel::IsClampedMin () const
{
    return m_clampInterval.minValue () > m_interval.minValue ();
}

bool ColorBarModel::IsClampedMax () const
{
    return m_clampInterval.maxValue () < m_interval.maxValue ();
}

void ColorBarModel::ColorMapCopy (const ColorBarModel& other)
{
    m_palette = other.m_palette;
    QwtDoubleInterval interval = GetInterval ();
    QwtDoubleInterval clampInterval = interval;
    double clampMin = other.GetClampMin ();
    double clampMax = other.GetClampMax ();
    if (other.IsClampedMin () && interval.contains (clampMin))
        clampInterval.setMinValue (clampMin);
    if (other.IsClampedMax () && interval.contains (clampMax))
        clampInterval.setMaxValue (clampMax);
    SetClampInterval (clampInterval);
}

float ColorBarModel::GetClampMinRatio () const
{
    return (m_clampInterval.minValue () - m_interval.minValue ()) / 
        (m_interval.maxValue () - m_interval.minValue ());
}

float ColorBarModel::GetClampMaxRatio () const
{
    return (m_clampInterval.maxValue () - m_interval.minValue ()) / 
        (m_interval.maxValue () - m_interval.minValue ());
}

G3D::Vector2 ColorBarModel::GetBarLabelSize () const
{
    StringWidth sr;
    ostringstream ostr;
    QwtDoubleInterval interval = GetInterval ();
    ostr << scientific << setprecision (1);

    ostr.str ("");ostr << GetTitle ();
    sr.AddString (ostr.str (), 
                 Settings::BAR_WIDTH + Settings::BAR_IN_BETWEEN_DISTANCE);
    ostr.str ("");ostr << interval.minValue ();
    sr.AddString (ostr.str ());
    ostr.str ("");ostr << interval.maxValue ();
    sr.AddString (ostr.str ());
    if (IsClampedMin ())
    {
        ostr.str ("");ostr << GetClampMin ();
        sr.AddString (ostr.str ());
    }
    if (IsClampedMax ())
    {
        ostr.str ("");ostr << GetClampMax ();
        sr.AddString (ostr.str ());
    }
    return G3D::Vector2 (sr.GetMaxWidth () + Settings::BAR_IN_BETWEEN_DISTANCE, 
                         sr.GetHeight ());
}
