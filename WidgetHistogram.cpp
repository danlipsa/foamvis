/**
 * @file   WidgetHistogram.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying histograms
 */

#include "AttributeHistogram.h"
#include "BodySelector.h"
#include "DebugStream.h"
#include "Foam.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"
#include "WidgetHistogram.h"

WidgetHistogram::WidgetHistogram(QWidget *parent)
    : QWidget (parent), 
      WidgetBase (this, &Settings::IsHistogramShown, 
                  &Settings::GetHistogramCount)
{
    fill (m_histogram.begin (), m_histogram.end (), (AttributeHistogram*)0);
}

int WidgetHistogram::GetHeight () const
{
    return m_histogram[0]->sizeHint ().height ();
}

void WidgetHistogram::Init (boost::shared_ptr<Settings> settings, 
                            const SimulationGroup* simulationGroup)
{    
    WidgetBase::Init (settings, simulationGroup, 0);
    m_signalMapperSelectionChanged.reset (new QSignalMapper (this));
    QLayout* layout = new QHBoxLayout ();
    for (size_t i = 0; i < m_histogram.size (); ++i)
    {
        m_histogram[i] = new AttributeHistogram (this);
        m_histogram[i]->setHidden (true);
        connect (
            m_histogram[i], 
            SIGNAL (SelectionChanged ()),
            m_signalMapperSelectionChanged.get (), 
            SLOT (map ()));
        m_signalMapperSelectionChanged->setMapping (m_histogram[i], i);
        layout->addWidget (m_histogram[i]);
    }
    connect (
        m_signalMapperSelectionChanged.get (),
        SIGNAL (mapped (int)),
        this, 
        SLOT (selectionChanged (int)));

    layout->setContentsMargins (0, 0, 0, 0);
    setLayout (layout);
    updateGeometry ();
}

void WidgetHistogram::mousePressEvent(QMouseEvent *event)
{
    QWidget *child = childAt (event->pos ());
    ForAllViews (
        boost::bind (&WidgetHistogram::setView, this, _1, child));
}
void WidgetHistogram::setView (ViewNumber::Enum viewNumber, QWidget* widget)
{
    if (m_histogram[viewNumber]->canvas () == widget)
        GetSettings ()->SetViewNumber (viewNumber);
}


void WidgetHistogram::selectionChanged (int viewNumber)
{
    Q_EMIT SelectionChanged (viewNumber);
}

void WidgetHistogram::UpdateSelection (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    const BodySelector* selector = &vs.GetBodySelector ();
    if (selector->GetType  () == BodySelectorType::COMPOSITE ||
        selector->GetType  () == BodySelectorType::PROPERTY_VALUE)
    {
        if (selector->GetType () == BodySelectorType::COMPOSITE)
            selector = static_cast<const CompositeBodySelector*> (selector)
                ->GetPropertyValueSelector ().get ();
        const vector<pair<size_t, size_t> >& v = 
            static_cast<const PropertyValueBodySelector*> (selector)->GetBins ();
        GetHistogram (viewNumber).SetSelectedBinsNoSignal (v);
    }
}

void WidgetHistogram::UpdateFocus ()
{
    ForAllViews (
        boost::bind (&WidgetHistogram::updateFocus, this, _1));
}
void WidgetHistogram::updateFocus (ViewNumber::Enum viewNumber)
{
    m_histogram[viewNumber]->DisplayFocus (
        viewNumber == GetSettings ()->GetViewNumber () && 
        GetSettings ()->IsViewFocusShown () &&
        GetSettings ()->GetViewCount () != ViewCount::ONE);
}


void WidgetHistogram::Update (boost::shared_ptr<ColorBarModel> colorBarModel,
                              SelectionOperation histogramSelection, 
                              MaxValueOperation maxValueOperation)
{
    Update (colorBarModel, histogramSelection, maxValueOperation, 
            GetSettings ()->GetViewNumber ());
}



void WidgetHistogram::Update (boost::shared_ptr<ColorBarModel> colorBarModel,
                              SelectionOperation histogramSelection, 
                              MaxValueOperation maxValueOperation, 
                              ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    bool colorMapped = vs.HasHistogramOption (HistogramType::COLOR_MAPPED);
    BodyScalar::Enum property = BodyScalar::FromSizeT (
        vs.GetBodyOrFaceScalar ());
    const Simulation& simulation = GetSimulation (viewNumber);

    m_histogram[viewNumber]->SetColorCoded (colorMapped);
    if (colorMapped)
        m_histogram[viewNumber]->SetColorTransferFunction (colorBarModel);

    double maxYValue = 0;
    QwtIntervalData intervalData;
    if (vs.HasHistogramOption (HistogramType::ALL_TIME_STEPS_SHOWN))
    {
        const HistogramStatistics& allTimestepsHistogram = 
            simulation.GetHistogram (property);
        intervalData = allTimestepsHistogram.ToQwtIntervalData ();
        maxYValue = allTimestepsHistogram.GetMaxCountPerBin ();
    }
    else
    {
        intervalData = simulation.GetFoam (GetSettings ()->GetCurrentTime ()).
            GetHistogram (property).ToQwtIntervalData ();
        maxYValue = simulation.GetMaxCountPerBinIndividual (property);
    }
    if (maxValueOperation == KEEP_MAX_VALUE)
    {
        // keep max value only if it was set before
        double myv = m_histogram[viewNumber]->GetMaxValueYAxis ();
        if (myv != 0)
            maxYValue = myv;
    }
    if (histogramSelection == KEEP_SELECTION)
        m_histogram[viewNumber]->SetDataKeepBinSelection (
            intervalData, maxYValue, BodyScalar::ToString (property));
    else
        m_histogram[viewNumber]->SetDataAllBinsSelected (
            intervalData, maxYValue, BodyScalar::ToString (property));
    m_histogram[viewNumber]->setVisible (vs.IsHistogramShown ());
    UpdateFocus ();
}

void WidgetHistogram::UpdateHidden ()
{
    ForAllHiddenViews (
        boost::bind (&WidgetHistogram::updateHidden, this, _1));
}
void WidgetHistogram::updateHidden (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    vs.SetHistogramShown (false);
    m_histogram[viewNumber]->setVisible (false);    
}


void WidgetHistogram::CurrentIndexChangedInteractionMode (int index)
{
    ForAllViews (
        boost::bind (&WidgetHistogram::currentIndexChangedInteractionMode, 
                     this, _1, index));
}
void WidgetHistogram::currentIndexChangedInteractionMode (
    ViewNumber::Enum viewNumber, int index)
{
    m_histogram[viewNumber]->CurrentIndexChangedInteractionMode (index);
}


void WidgetHistogram::SetHeight (int h)
{
    ForAllViews (
        boost::bind (&WidgetHistogram::setHeight, this, _1, h));
}
void WidgetHistogram::setHeight (ViewNumber::Enum viewNumber, int h)
{
    AttributeHistogram* histogram = m_histogram[viewNumber];
    QSize size = histogram->sizeHint ();
    size.setHeight (h);
    histogram->SetSizeHint (size);
    histogram->updateGeometry ();
}

void WidgetHistogram::SetGridShown (bool shown)
{
    ForAllViews (
        boost::bind (&WidgetHistogram::setGridShown, this, _1, shown));
}
void WidgetHistogram::setGridShown (ViewNumber::Enum viewNumber, bool shown)
{
    m_histogram[viewNumber]->SetGridEnabled (shown);    
}

void WidgetHistogram::SetDefaultFont ()
{
    ForAllViews (
        boost::bind (&WidgetHistogram::setDefaultFont, this, _1));
}
void WidgetHistogram::setDefaultFont (ViewNumber::Enum viewNumber)
{
    m_histogram[viewNumber]->SetDefaultFont ();
}



