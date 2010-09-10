/**
 * @file   ColorBarModel.cpp
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Definition of the ColorBarModel class
 */

#include "ColorBarModel.h"
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

class RainbowTeleaMapper
{
public:
    QColor operator() (double value);
};

class ColorTransferFunctionMapper
{
public:
    ColorTransferFunctionMapper (
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
    RainbowTeleaMapper rainbowTeleaMapper;
    for (size_t i = 0; i <= 12; i++)
    {
	double value = static_cast<double>(i) / 2;
	cdbg << setw (3) << value << " " << rainbowTeleaMapper (value/6) << endl;
    }
}


const size_t ColorBarModel::COLORS = 256;

ColorBarModel::ColorBarModel () :
    m_palette (Palette::FIRST),
    m_interval (0, 1),
    m_clampValues (0, 1)
{
}


void ColorBarModel::setupPaletteRainbowTelea ()
{
    setupColorMap (RainbowTeleaMapper ());
}

void ColorBarModel::SetupPalette (Palette::Enum palette)
{
    m_palette = palette;
    switch (palette)
    {
    case Palette::RAINBOW_TELEA:
	setupPaletteRainbowTelea ();
	break;
    case Palette::RAINBOW_HSV:
	setupPaletteRainbowHSV ();
	break;
    case Palette::BLACK_BODY:
	setupPaletteBlackBody ();
	break;
    case Palette::BLUE_RED_DIVERGING:
    case Palette::BLUE_TAN_DIVERGING:
    case Palette::PURPLE_ORANGE_DIVERGING:
    case Palette::GREEN_PURPLE_DIVERGING:
    case Palette::GREEN_RED_DIVERGING:
	setupPaletteDiverging (palette - Palette::BLUE_RED_DIVERGING);
	break;
    }
}

void ColorBarModel::setupPaletteBlackBody ()
{
  VTK_CREATE(vtkColorTransferFunction, blackbody);
  blackbody->SetColorSpaceToRGB();
  blackbody->AddRGBPoint(0.0, 0.0, 0.0, 0.0);   // black
  blackbody->AddRGBPoint(0.4, 0.9, 0.0, 0.0);   // red
  blackbody->AddRGBPoint(0.8, 0.9, 0.9, 0.0);   // yellow
  blackbody->AddRGBPoint(1.0, 1.0, 1.0, 1.0);   // white
  setupColorMap (ColorTransferFunctionMapper (blackbody));
}

void ColorBarModel::setupPaletteRainbowHSV ()
{
  VTK_CREATE(vtkColorTransferFunction, rainbow);
  rainbow->SetColorSpaceToHSV();
  rainbow->HSVWrapOff();
  rainbow->AddHSVPoint(0.0, 0.66667, 1.0, 1.0); // blue
  rainbow->AddHSVPoint(1.0, 0.0, 1.0, 1.0);     // red
  setupColorMap (ColorTransferFunctionMapper (rainbow));
}



void ColorBarModel::setupPaletteDiverging (size_t c)
{
    const static double colors[][2][3] = 
	{
	    /* blue red*/
	    {{0.230, 0.299, 0.754},
	     {0.706, 0.016, 0.150}},
	    /* blue to tan */
	    {{0.217, 0.525, 0.910},
	     {0.677, 0.492, 0.093}},
	    /* purple to orange */
	    {{0.436, 0.308, 0.631},
	     {0.759, 0.334, 0.046}},
	    /* green to purple */
	    {{0.085, 0.532, 0.201},
	     {0.436, 0.308, 0.631}},
	    /* green to red */
	    {{0.085, 0.532, 0.201},
	     {0.758, 0.214, 0.233}}
	};
    
    VTK_CREATE(vtkColorTransferFunction, colorTransferFunction);
    colorTransferFunction->SetColorSpaceToDiverging();
    colorTransferFunction->AddRGBPoint(
	0.0, colors[c][0][0], colors[c][0][1], colors[c][0][2]);
    colorTransferFunction->AddRGBPoint(
	1.0, colors[c][1][0], colors[c][1][1], colors[c][1][2]);
    setupColorMap (ColorTransferFunctionMapper (colorTransferFunction));
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



/*
 * @return 1 if value is between begin and end,
 *         0 if value is less than begin - 1 or greater than end + 1
 *         value linearly scaled between [0, 1] if value is in [begin - 1, begin]
 *         or [end, end+1]
 */
double trapezoid (double value, double begin, double end)
{
    double color = max (
	0.0,
	((end - begin + 2) - (abs (value - begin) + abs (value - end))) / 2);
    return color;
}

QColor RainbowTeleaMapper::operator() (double f)
{
    const double dx = .8;
    QColor color;

    f = (f < 0) ? 0 : (f > 1) ? 1 : f; // clamp f in [0, 1]
    double g = (6 - 2*dx) * f + dx;    // scale f to [dx, 6 - dx]

    color.setRedF (trapezoid (g, 4, 5));
    color.setGreenF (trapezoid (g, 2, 4));
    color.setBlueF (trapezoid (g, 1, 2));
    return color;
}
