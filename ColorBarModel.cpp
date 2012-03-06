/**
 * @file   ColorBarModel.cpp
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Definition of the ColorBarModel class
 */

#include "ColorBarModel.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Utils.h"

/**
 * Implementation of the Rainbow Colormap from 
 * Data Visualization, Principles and Practice,
 * Alexandru C. Telea
 * Section 5.2 Designing Effective Colormaps, page 132
 * Returns the rainbow color coresponding o a value between 0 and 1, 
 * blue=0, green=.5 red=1
 *
 */

class ColorMapperRainbowTelea
{
public:
    QColor operator() (double value);
};

class ColorMapperVtkColorTransferFunction
{
public:
    ColorMapperVtkColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction) :
	m_colorTransferFunction (colorTransferFunction)
    {
    }

    QColor operator () (double value)
    {
	QColor color;
	color.setRedF (m_colorTransferFunction->GetRedValue (value));
	color.setGreenF (m_colorTransferFunction->GetGreenValue (value));
	color.setBlueF (m_colorTransferFunction->GetBlueValue (value));
	return color;
    }

private:
    vtkSmartPointer<vtkColorTransferFunction> m_colorTransferFunction;
};

void testColorMap ()
{
    cdbg << "Test Color map:" << endl;
    ColorMapperRainbowTelea rainbowTeleaMapper;
    for (size_t i = 0; i <= 12; i++)
    {
	double value = static_cast<double>(i) / 2;
	cdbg << setw (3) << value << " " << rainbowTeleaMapper (value/6) << endl;
    }
}


const size_t ColorBarModel::COLORS = 256;

ColorBarModel::ColorBarModel () :
    m_image (COLORS, 1, QImage::Format_RGB32),
    m_interval (0, 1),
    m_clampValues (0, 1)
{
}

QColor ColorBarModel::GetHighlightColor (HighlightNumber::Enum i) const
{
    return m_highlightColor[i];
}

void ColorBarModel::setupPaletteRainbow ()
{
    VTK_CREATE(vtkColorTransferFunction, rainbow);
    rainbow->SetColorSpaceToLab();
    rainbow->AddRGBPoint(0.0, 0, 0, 1);   // blue
    rainbow->AddRGBPoint(0.25, 0, 1, 1);   // cyan
    rainbow->AddRGBPoint(0.50, 0, 1, 0);   // green
    rainbow->AddRGBPoint(0.75, 1, 1, 0);   // yellow
    rainbow->AddRGBPoint(1.0, 1, 0, 0);   // red
    setup (ColorMapperVtkColorTransferFunction (rainbow));
    m_highlightColor[0] = Qt::black;
    m_highlightColor[1] = Qt::yellow;
    m_highlightColor[2] = Qt::white;
}

void ColorBarModel::setupPaletteRainbowExtended ()
{
    VTK_CREATE(vtkColorTransferFunction, rainbow);
    rainbow->SetColorSpaceToLab();
    rainbow->AddRGBPoint(0.0, 1, 0, 1);   // magenta
    rainbow->AddRGBPoint(0.2, 0, 0, 1);   // blue
    rainbow->AddRGBPoint(0.4, 0, 1, 1);   // cyan
    rainbow->AddRGBPoint(0.6, 0, 1, 0);   // green
    rainbow->AddRGBPoint(0.8, 1, 1, 0);   // yellow
    rainbow->AddRGBPoint(1.0, 1, 0, 0);   // red
    setup (ColorMapperVtkColorTransferFunction (rainbow));    
    m_highlightColor[0] = Qt::black;
    m_highlightColor[1] = Qt::yellow;
    m_highlightColor[2] = Qt::white;
}



void ColorBarModel::setupPaletteRainbowTelea ()
{
    setup (ColorMapperRainbowTelea ());
    m_highlightColor[0] = Qt::black;
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
  VTK_CREATE(vtkColorTransferFunction, blackbody);
  blackbody->SetColorSpaceToLab();
  blackbody->AddRGBPoint(0.0, 0, 0, 0);   // black
  blackbody->AddRGBPoint(0.33, 1, 0, 0);   // red
  blackbody->AddRGBPoint(0.66, 1, 1, 0);   // yellow
  blackbody->AddRGBPoint(1.0, 1, 1, 1);   // white
  setup (ColorMapperVtkColorTransferFunction (blackbody));
  m_highlightColor[0] = Qt::green;
  m_highlightColor[1] = Qt::blue;
  m_highlightColor[2] = Qt::magenta;
}

void ColorBarModel::setupPaletteSequentialBrewerBlues9 ()
{
    VTK_CREATE(vtkColorTransferFunction, f);
    f->SetColorSpaceToLab();
    f->AddRGBPoint(0.0    , 0.968627, 0.984314, 1.000000);
    //f->AddRGBPoint(0.0    ,        1,        1, 1.000000); // change to white
    f->AddRGBPoint(0.12500, 0.870588, 0.921569, 0.968627);
    f->AddRGBPoint(0.25000, 0.776471, 0.858824, 0.937255);
    f->AddRGBPoint(0.37500, 0.619608, 0.792157, 0.882353);
    f->AddRGBPoint(0.50000, 0.419608, 0.682353, 0.839216);
    f->AddRGBPoint(0.62500, 0.258824, 0.572549, 0.776471);
    f->AddRGBPoint(0.75000, 0.129412, 0.443137, 0.709804);
    f->AddRGBPoint(0.87500, 0.031373, 0.317647, 0.611765);
    f->AddRGBPoint(1.00000, 0.031373, 0.188235, 0.419608);
    setup (ColorMapperVtkColorTransferFunction (f));
    m_highlightColor[0] = Qt::black;
    m_highlightColor[1] = Qt::red;
    m_highlightColor[2] = Qt::green;
}

void ColorBarModel::setupPaletteSequentialBrewerYlOrRd9 ()
{
    VTK_CREATE(vtkColorTransferFunction, f);
    f->SetColorSpaceToLab();
    f->AddRGBPoint(0.0    , 1.00000, 1.00000, 0.80000);
    f->AddRGBPoint(0.12500, 1.00000, 0.92941, 0.62745);
    f->AddRGBPoint(0.25000, 0.99608, 0.85098, 0.46275);
    f->AddRGBPoint(0.37500, 0.99608, 0.69804, 0.29804);
    f->AddRGBPoint(0.50000, 0.99216, 0.55294, 0.23529);
    f->AddRGBPoint(0.62500, 0.98824, 0.30588, 0.16471);
    f->AddRGBPoint(0.75000, 0.89020, 0.10196, 0.10980);
    f->AddRGBPoint(0.87500, 0.74118, 0.00000, 0.14902);
    f->AddRGBPoint(1.00000, 0.50196, 0.00000, 0.14902);
    setup (ColorMapperVtkColorTransferFunction (f));
    m_highlightColor[0] = Qt::black;
    m_highlightColor[1] = Qt::blue;
    m_highlightColor[2] = Qt::green;
}




void ColorBarModel::setupPaletteRainbowHSV ()
{
  VTK_CREATE(vtkColorTransferFunction, rainbow);
  rainbow->SetColorSpaceToHSV();
  rainbow->HSVWrapOff();
  rainbow->AddHSVPoint(0.0, 0.66667, 1.0, 1.0); // blue
  rainbow->AddHSVPoint(1.0, 0.0, 1.0, 1.0);     // red
  setup (ColorMapperVtkColorTransferFunction (rainbow));
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
	     {0, 0, 0}, {0, 1, 1}, {0, 1, 0} // black, cyan, green
	    },

	    /* blue to tan */
	    {{0.217, 0.525, 0.910},
	     {0.677, 0.492, 0.093},
	     {0, 0, 0}, {1, 0, 0}, {0, 1, 0}
	    },

	    /* purple to orange */
	    {{0.436, 0.308, 0.631},
	     {0.759, 0.334, 0.046},
	     {0, 0, 0}, {1, 1, 0}, {0, 0, 1}
	    },

	    /* green to purple */
	    {{0.085, 0.532, 0.201},
	     {0.436, 0.308, 0.631},
	     {0, 0, 0}, {1, 1, 0}, {1, 0, 0}
	    },

	    /* green to red */
	    {{0.085, 0.532, 0.201},
	     {0.758, 0.214, 0.233},
	     {0, 0, 0}, {1, 1, 0}, {0, 0, 1}
	    }
	};    
    VTK_CREATE(vtkColorTransferFunction, colorTransferFunction);
    colorTransferFunction->SetColorSpaceToDiverging();
    colorTransferFunction->AddRGBPoint(
	0.0, colors[c][0][0], colors[c][0][1], colors[c][0][2]);
    colorTransferFunction->AddRGBPoint(
	1.0, colors[c][1][0], colors[c][1][1], colors[c][1][2]);
    setup (ColorMapperVtkColorTransferFunction (colorTransferFunction));
    for (size_t i = 0; i < m_highlightColor.size (); ++i)
	m_highlightColor[i] = QColor::fromRgbF (
	    colors[c][i+2][0], colors[c][i+2][1], colors[c][i+2][2]);
}

template<typename ColorMapper>
void ColorBarModel::setup (ColorMapper colorMapper)
{
    setupColorMap (colorMapper);
    setupImage (colorMapper);
}


template<typename ColorMapper>
void ColorBarModel::setupColorMap (ColorMapper colorMapper)
{
    m_colorMap.setColorInterval (colorMapper (0), colorMapper (1));
    double width = m_interval.width ();
    double low = (m_clampValues.minValue () -  m_interval.minValue ()) / width;
    double high = (m_clampValues.maxValue () -  m_interval.minValue ()) / width;
    if (low != 0)
	m_colorMap.addColorStop (low, colorMapper (0));
    if (high != 1)
	m_colorMap.addColorStop (high, colorMapper (1));
    m_colorMap.setMode (QwtLinearColorMap::FixedColors);
    size_t colors = COLORS - 1;
    for (size_t i = 1; i < colors; ++i)
    {
	double value = static_cast<double>(i) / colors;
	double insideClamp = low + value * (high - low);
	m_colorMap.addColorStop (insideClamp, colorMapper (value));
    }
}


template<typename ColorMapper>
void ColorBarModel::setupImage (ColorMapper colorMapper)
{
    double width = m_interval.width ();
    double low = (m_clampValues.minValue () -  m_interval.minValue ()) / width;
    double high = (m_clampValues.maxValue () -  m_interval.minValue ()) / width;
    size_t colors = COLORS - 1;
    for (size_t i = 0; i <= colors; i++)
    {
	double value = static_cast<double>(i) / colors;
	uint rgb;
	if (value <= low)
	    rgb = static_cast<uint> (
		colorMapper (0).rgb ());
	else if (value >= high)
	    rgb = static_cast<uint> (
		colorMapper (1).rgb ());
	else
	{
	    double inside01 = (value - low) / (high - low);
	    rgb = static_cast<uint> (
		colorMapper (inside01).rgb ());
	}
	m_image.setPixel (i, 0, rgb);
    }
}

/*
 * @return 1 if value is between heigh1 and heigh2
 *         0 if value is less than low1 or greater than heigh2
 *         value linearly scaled between [0, 1] if value is in [low1, heigh1]
 *         or [heigh2, low2]
 */
double trapezoid (
    double value,
    double low1, double heigh1, double heigh2, double low2)
{
    if (value <= low1 || value >= low2)
	return 0;
    if (value > low1 && value < heigh1)
	return (value - low1) / (heigh1 - low1);
    if (value > heigh2 && value < low2)
	return (low2 - value) / (low2 - heigh2);
    return 1;
}

QColor ColorMapperRainbowTelea::operator() (double f)
{
    const double dx = 1;
    QColor color;

    f = (f < 0) ? 0 : (f > 1) ? 1 : f; // clamp f in [0, 1]
    double g = (6 - 2*dx) * f + dx;    // scale f to [dx, 6 - dx]

    color.setBlueF (trapezoid (g, 0, 1, 2.2, 3));
    color.setGreenF (trapezoid (g, 1, 2.2, 3.8, 5));
    color.setRedF (trapezoid (g, 3, 3.8, 5, 6));
    return color;
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
    ostr << "ColorBarModel: " << m_interval;
    return ostr.str ();
}
