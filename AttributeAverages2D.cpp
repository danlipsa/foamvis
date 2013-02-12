/**
 * @file   AttributeAverages2D.cpp
 * @author Dan R. Lipsa
 * @date 19 July 2012
 *
 * Definitions for the view average
 */

#include "ForceAverage.h"
#include "ScalarAverage.h"
#include "Simulation.h"
#include "T1sKDE.h"
#include "TensorAverage.h"
#include "VectorAverage.h"
#include "AttributeAverages2D.h"
#include "WidgetGl.h"


// Private Functions
// ======================================================================




// Methods
// ======================================================================

AttributeAverages2D::AttributeAverages2D (ViewNumber::Enum viewNumber,
                                          const WidgetGl& widgetGl) :
    AttributeAverages (viewNumber, *widgetGl.GetSettings (),
                       widgetGl.GetSimulationGroup ())
{
    m_scalarAverage.reset (new ScalarAverage (viewNumber, widgetGl));
    m_t1sKDE.reset (new T1sKDE (viewNumber, widgetGl));
    m_deformationAverage.reset (
	new TensorAverage (
            viewNumber, widgetGl, GetScalarAverage ().GetFbos ()));
    m_velocityAverage.reset (
	new VectorAverage (viewNumber, widgetGl));
}

