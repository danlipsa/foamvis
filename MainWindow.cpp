/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the MainWindow
 */
// @todo Store the palette per BodyScalar instead of storing it per view 
//       and simulation index.
#include "Application.h"
#include "AttributeHistogram.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "ScalarAverage.h"
#include "Debug.h"
#include "EditColorMap.h"
#include "Foam.h"
#include "Simulation.h"
#include "WidgetGl.h"
#include "MainWindow.h"
#include "ProcessBodyTorus.h"
#include "Settings.h"
#include "SystemDifferences.h"
#include "T1sKDE.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "OpenGLUtils.h"
#include "RegularGridAverage.h"
#include "VectorAverage.h"
#include "AttributeAverages2D.h"
#include "ViewSettings.h"


// Private Functions
// ======================================================================

int checkedId (const QButtonGroup* group)
{
    return group->id (group->checkedButton ());
}

template<size_t size>
void setVisible (const boost::array<QWidget*, size>& widgets, bool visible)
{
    BOOST_FOREACH(QWidget* widget, widgets)
	widget->setVisible (visible);
}

template<size_t size>
void setEnabled (const boost::array<QWidget*, size>& widgets, bool enabled)
{
    BOOST_FOREACH(QWidget* widget, widgets)
	widget->setEnabled (enabled);
}

// Static fields

const char* MainWindow::PLAY_FORWARD_TEXT (">");
const char* MainWindow::PLAY_REVERSE_TEXT ("<");
const char* MainWindow::PAUSE_TEXT ("||");

// Methods
// ======================================================================

MainWindow::MainWindow (
    boost::shared_ptr<const SimulationGroup> simulationGroup) : 
    m_timer (new QTimer(this)),
    m_processBodyTorus (0), 
    m_debugTranslatedBody (false),
    m_currentBody (0),
    m_editColorMap (new EditColorMap (this)),
    m_playForward (false),
    m_playReverse (false)
{
    SetSimulationGroup (simulationGroup);
    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
        m_averageCache[i].reset (new AverageCache ());
    // for anti-aliased lines
    QGLFormat format = QGLFormat::defaultFormat ();
    format.setSampleBuffers (true);
    format.setAlpha (true);
    QGLFormat::setDefaultFormat(format);
    
    setupUi (this);
    SetSettings(boost::shared_ptr<Settings> (
                    new Settings (simulationGroup, 
                                  widgetGl->width (), widgetGl->height ())));
    connect (GetSettingsPtr ().get (),
             SIGNAL (SelectionChanged (ViewNumber::Enum)),
             this,
             SLOT (SelectionChangedSettings (ViewNumber::Enum)));
    widgetHistogram->Init (GetSettingsPtr (), simulationGroup);
    connect (
	widgetHistogram,
	SIGNAL (SelectionChanged (int)),
	this, 
	SLOT (SelectionChangedHistogram (int)));
    
    connectSignals ();
    setupButtonGroups ();


    widgetGl->Init (GetSettingsPtr (), simulationGroup, &m_averageCache);
    widgetGl->SetStatus (labelStatusBar);

    widgetVtk->Init (GetSettingsPtr (), simulationGroup);
    size_t index3DSimulation = simulationGroup->GetIndex3DSimulation ();
    if (index3DSimulation != INVALID_INDEX)
    {
	const Foam& foam = GetSimulation (index3DSimulation).GetFoam (0);
        boost::shared_ptr<Application> app = Application::Get ();
        QFont defaultFont = app->font ();
	widgetVtk->Average3dCreatePipeline (
	    foam.GetObjects ().size (), foam.GetConstraintFaces ().size (), 
	    defaultFont.pointSize ());
    }
    setupColorBarModels ();
    setupViews ();
    initComboBoxSimulation (*simulationGroup);
    configureInterface ();
    createActions ();
    setTabOrder (radioButtonBubblesPaths, sliderTimeSteps);
    string title ("FoamVis");
    if (simulationGroup->size () == 1)
    {
	const Simulation& simulation = GetSimulation (0);
	title += " - ";
	title += simulation.GetName ();
    }
    setWindowTitle (QString (title.c_str ()));

    spinBoxHistogramHeight->setMaximum (500);
    spinBoxHistogramHeight->setValue (widgetHistogram->GetHeight ());

    m_timer->setInterval (20);
    //initTranslatedBody ();
    configureInterfaceDataDependent (*simulationGroup);    
    ValueChangedSliderTimeSteps (0);
    ButtonClickedViewType (ViewType::FACES);
    widgetSave->resize (720, 480);
    widgetSave->updateGeometry ();
}

void MainWindow::configureInterface ()
{
    horizontalSliderForceSize->setValue (49);
    horizontalSliderTorqueDistance->setValue (49);
    comboBoxColor->setCurrentIndex (BodyScalar::PRESSURE);
    CurrentIndexChangedInteractionMode (InteractionMode::ROTATE);
    CurrentIndexChangedWindowLayout (ViewLayout::HORIZONTAL);
    doubleSpinBoxKernelSigma->setToolTip (
        "Standard deviation in bubble diameters.");
    doubleSpinBoxKDEValue->setToolTip (
        "Oversample streamlines for grid cells where the "
        "center's KDE value is greater than the entered KDE value.");
    spinBoxKDEMultiplier->setToolTip (
        "Oversample streamlines. A grid square will "
        "contain (2*m + 1)^2 seeds where m is the KDE multiplier.");
    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    spinBoxFontSize->setValue (defaultFont.pointSize ());
}


void MainWindow::configureInterfaceDataDependent (
    const SimulationGroup& simulationGroup)
{
    const Simulation& simulation = simulationGroup.GetSimulation (0);
    setupSliderData (simulation);
    if (simulation.IsTopologicalChangeAvailable ())
    {
	checkBoxT1sShown->setEnabled (true);	
	radioButtonT1sKDE->setEnabled (true);
    }
    if (! simulation.IsTorus ())
    {
	checkBoxBoundingBoxTorusDomain->setDisabled (true);
	checkBoxDomainClipped->setDisabled (true);
        checkBoxDomainTop->setDisabled (true);
        checkBoxDomainBottom->setDisabled (true);
        checkBoxDomainLeft->setDisabled (true);
        checkBoxDomainRight->setDisabled (true);
	radioButtonEdgesTorus->setDisabled (true);
	radioButtonFaceEdgesTorus->setDisabled (true);
    }
    if (simulation.Is2D ())
    {
        horizontalSliderT1Size->setValue (49);
	comboBoxInteractionMode->setCurrentIndex (InteractionMode::SCALE);
	if (simulation.IsTorus ())
	{
	    G3D::AABox box = simulation.GetBoundingBox ();
	    G3D::Vector3 extent = box.extent ();
	    if (extent.x > extent.y && simulation.GetRotation2D () == 0)
		comboBoxViewLayout->setCurrentIndex (ViewLayout::VERTICAL);
	}
    }
    else
    {
	if (simulation.GetRegularGridResolution () == 0)
	    radioButtonAverage->setDisabled (true);
	comboBoxAxisOrder->setCurrentIndex (AxisOrderName::THREE_D);
	comboBoxColor->setItemText (
	    BodyScalar::SIDES_PER_BUBBLE, 
	    BodyScalar::ToString (BodyScalar::SIDES_PER_BUBBLE));
	comboBoxColor->setItemText (
	    BodyScalar::DEFORMATION_SIMPLE,
	    BodyScalar::ToString (BodyScalar::DEFORMATION_SIMPLE));
    }
    size_t viewCount = min (simulationGroup.size (), 
			    size_t (ViewCount::COUNT - 1));
    if (simulationGroup.size () > 1)
	comboBoxViewCount->setCurrentIndex (viewCount - 1);
    for (size_t i = 1; i < viewCount; ++i)
    {
        ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	GetSettingsPtr ()->SetViewNumber (viewNumber);
	comboBoxSimulation->setCurrentIndex (i);
    }
    initOverlayBarModel ();
}


void MainWindow::initComboBoxSimulation (const SimulationGroup& simulationGroup)
{
    size_t simulationsCount = simulationGroup.size ();
    if (simulationsCount == 1)
	comboBoxSimulation->setHidden (true);
    else
    {
	comboBoxSimulation->blockSignals (true);
	for (size_t i = 0; i < simulationsCount; ++i)
	{
	    const Simulation& simulation = 
		simulationGroup.GetSimulation (i);
	    comboBoxSimulation->addItem (
		simulation.GetName ().c_str ());
	}
	comboBoxSimulation->blockSignals (false);
    }
}

void MainWindow::translatedBodyInit ()
{
    m_debugTranslatedBody = true;
    Foam& foam = const_cast<Foam&>(GetSimulation ().GetFoam (0));
    m_currentTranslatedBody = foam.GetBodies ().begin ();
}


void MainWindow::connectSignals ()
{
    
    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));
    
    connect (widgetGl, SIGNAL (PaintEnd ()),
	     widgetSave, SLOT (SaveFrame ()), 
	     Qt::QueuedConnection);

    connect (widgetVtk, SIGNAL (PaintEnd ()),
	     widgetSave, SLOT (SaveFrame ()), 
	     Qt::QueuedConnection);

    connect (tableWidgetEvents, SIGNAL (cellClicked (int, int)),
             this, SLOT (CellClickedLinkedTimeEvents (int, int)));

    
    // BodyOrFaceScalarChanged: 
    // from MainWindow to WidgetGl
    connect (
	this, 
	SIGNAL (BodyOrFaceScalarChanged (
		    ViewNumber::Enum,
		    boost::shared_ptr<ColorBarModel>, size_t)),
	widgetGl, 
	SLOT (SetBodyOrFaceScalar (
		  ViewNumber::Enum, boost::shared_ptr<ColorBarModel>, size_t)));
    
    // ColorBarModelChanged & OverlayBarModelChanged
    // from MainWindow to WidgetGl
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (ViewNumber::Enum, 
				      boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (SetColorBarModel (ViewNumber::Enum, 
				boost::shared_ptr<ColorBarModel>)));
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (ViewNumber::Enum, 
				      boost::shared_ptr<ColorBarModel>)),
	widgetVtk, 
	SLOT (FromView (ViewNumber::Enum)));
    connect (
	this, 
	SIGNAL (OverlayBarModelChanged (ViewNumber::Enum, 
					boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (SetOverlayBarModel (ViewNumber::Enum, 
					  boost::shared_ptr<ColorBarModel>)));
    connect (
	this, 
	SIGNAL (OverlayBarModelChanged (ViewNumber::Enum, 
					boost::shared_ptr<ColorBarModel>)),
	widgetVtk, 
	SLOT (FromView (ViewNumber::Enum)));

        
    connectColorBarHistogram (true);
    
    connect (
	GetSettingsPtr ().get (),
	SIGNAL (ViewChanged (ViewNumber::Enum)),
	this,
	SLOT (ViewToUI (ViewNumber::Enum)));
}

void MainWindow::connectColorBarHistogram (bool connected)
{
    if (connected)
    {
	connect (
	    this, 
	    SIGNAL (BodyOrFaceScalarChanged (
			ViewNumber::Enum,
			boost::shared_ptr<ColorBarModel>, size_t)),
	    this, 
	    SLOT (SetHistogramColorBarModel (
		      ViewNumber::Enum,
		      boost::shared_ptr<ColorBarModel>)));
	connect (
	    this, 
	    SIGNAL (ColorBarModelChanged (ViewNumber::Enum,
					  boost::shared_ptr<ColorBarModel>)),
	    this, 
	    SLOT (SetHistogramColorBarModel (ViewNumber::Enum,
					     boost::shared_ptr<ColorBarModel>)),
	    Qt::UniqueConnection);
    }
    else
    {
	disconnect (
	    this, 
	    SLOT (SetHistogramColorBarModel (
		      boost::shared_ptr<ColorBarModel>)));
    }
}



void MainWindow::setupButtonGroups ()
{        
    buttonGroupViewType->setId (radioButtonEdgesNormal, ViewType::EDGES);
    buttonGroupViewType->setId (radioButtonEdgesTorus, ViewType::EDGES_TORUS);
    buttonGroupViewType->setId (radioButtonFaceEdgesTorus, 
				ViewType::FACES_TORUS);
    buttonGroupViewType->setId (radioButtonFacesNormal, ViewType::FACES);
    buttonGroupViewType->setId (radioButtonBubblesPaths, ViewType::CENTER_PATHS);
    buttonGroupViewType->setId (radioButtonAverage, 
				ViewType::AVERAGE);
    buttonGroupViewType->setId (radioButtonT1sKDE, ViewType::T1S_KDE);
    

    buttonGroupInteractionObject->setId (
	radioButtonInteractionFocus, InteractionObject::FOCUS);
    buttonGroupInteractionObject->setId (
	radioButtonInteractionLight, InteractionObject::LIGHT);
    buttonGroupInteractionObject->setId (
	radioButtonInteractionContext, InteractionObject::CONTEXT);
    buttonGroupInteractionObject->setId (
	radioButtonInteractionSeeds, InteractionObject::GRID);

    buttonGroupTimeLinkage->setId (
	radioButtonTimeIndependent, TimeLinkage::INDEPENDENT);
    buttonGroupTimeLinkage->setId (
	radioButtonTimeLinked, TimeLinkage::LINKED);
    buttonGroupTimeLinkage->button (TimeLinkage::LINKED)->setChecked (true);

    buttonGroupDuplicateDomain->setId (
        checkBoxDomainLeft, DuplicateDomain::LEFT);
    buttonGroupDuplicateDomain->setId (
        checkBoxDomainRight, DuplicateDomain::RIGHT);
    buttonGroupDuplicateDomain->setId (
        checkBoxDomainTop, DuplicateDomain::TOP);
    buttonGroupDuplicateDomain->setId (
        checkBoxDomainBottom, DuplicateDomain::BOTTOM);

    buttonGroupVelocityVis->setId (radioButtonVelocityGlyph, VectorVis::GLYPH);
    buttonGroupVelocityVis->setId (radioButtonVelocityStreamline, 
                                   VectorVis::STREAMLINE);
    buttonGroupVelocityVis->setId (radioButtonVelocityPathline, 
                                   VectorVis::PATHLINE);

    buttonGroupForce->setId (checkBoxForceNetwork, ForceType::NETWORK);
    buttonGroupForce->setId (checkBoxForcePressure, ForceType::PRESSURE);
    buttonGroupForce->setId (checkBoxForceResult, ForceType::RESULT);
    buttonGroupForce->setId (checkBoxForceDifference, ForceType::DIFFERENCE);

    buttonGroupTorque->setId (checkBoxTorqueNetwork, ForceType::NETWORK);
    buttonGroupTorque->setId (checkBoxTorquePressure, ForceType::PRESSURE);
    buttonGroupTorque->setId (checkBoxTorqueResult, ForceType::RESULT);
}

void MainWindow::setupSliderData (const Simulation& simulation)
{
    sliderTimeSteps->setMinimum (0);
    sliderTimeSteps->setMaximum (simulation.GetTimeSteps () - 1);
    sliderTimeSteps->setSingleStep (1);
    sliderTimeSteps->setPageStep (10);
}

void MainWindow::ResetTransformAll ()
{
    if (IsGlView (GetViewNumber ()))
        widgetGl->ResetTransformAll ();
    else
        widgetVtk->ResetTransformAll ();
}

void MainWindow::RotateShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::ROTATE);
}

void MainWindow::ScaleShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SCALE);
}

void MainWindow::TranslateShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::TRANSLATE);
}

void MainWindow::SelectShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SELECT);
}

void MainWindow::DeselectShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::DESELECT);
}


void MainWindow::updateButtons ()
{
    bool play = (m_playForward || m_playReverse);
    enableBegin (! play);
    enableEnd (! play);
    enablePlayForward ();
    enablePlayReverse ();
}

void MainWindow::enableBegin (bool enable)
{
    if (enable && sliderTimeSteps->value () > sliderTimeSteps->minimum ())
	toolButtonBegin->setEnabled (true);
    else
	toolButtonBegin->setDisabled (true);
}

void MainWindow::enableEnd (bool enable)
{
    if (enable && sliderTimeSteps->value () < sliderTimeSteps->maximum ())
	toolButtonEnd->setEnabled (true);
    else
	toolButtonEnd->setDisabled (true);
}

void MainWindow::enablePlayForward ()
{
    if (sliderTimeSteps->value () < sliderTimeSteps->maximum ())
	toolButtonPlayForward->setEnabled (true);
}

void MainWindow::enablePlayReverse ()
{
    if (sliderTimeSteps->value () > sliderTimeSteps->minimum ())
	toolButtonPlayReverse->setEnabled (true);
}


void MainWindow::keyPressEvent (QKeyEvent* event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers ();
    switch (event->key ())
    {
    case Qt::Key_B:
	translatedBodyStep ();
	break;
	
    case Qt::Key_A:
	processBodyTorusStep ();
	break;
	
    case Qt::Key_P:
	cdbg << "OpenGL State:" << endl;
	cdbg << G3D::getOpenGLState (false) << endl;
	break;
    }
}

void MainWindow::translatedBodyStep ()
{
    if (m_debugTranslatedBody)
    {
	Foam& currentFoam = const_cast<Foam&> (GetSimulation ().GetFoam (0));
	VertexSet vertexSet;
	EdgeSet edgeSet;
	FaceSet faceSet;
	currentFoam.GetVertexSet (&vertexSet);
	currentFoam.GetEdgeSet (&edgeSet);
	currentFoam.GetFaceSet (&faceSet);
	m_currentTranslatedBody = currentFoam.BodyInsideOriginalDomainStep (
	    m_currentTranslatedBody, &vertexSet, &edgeSet, &faceSet);
	if (m_currentTranslatedBody == currentFoam.GetBodies ().end ())
	{
	    cdbg << "End body translation" << endl;
	}
	widgetGl->CompileUpdate ();
    }
}

void MainWindow::processBodyTorusStep ()
{
    try
    {
	const Foam& currentFoam = GetSimulation ().GetFoam (0);
	boost::shared_ptr<Body>  b = currentFoam.GetBodyPtr (m_currentBody);
	if (m_processBodyTorus == 0)
	{
	    m_processBodyTorus = new ProcessBodyTorus (currentFoam, b);
	    m_processBodyTorus->Initialize ();
	}
	else
	{
	    VertexSet vertexSet;
	    EdgeSet edgeSet;
	    FaceSet faceSet;
	    currentFoam.GetVertexSet (&vertexSet);
	    currentFoam.GetEdgeSet (&edgeSet);
	    currentFoam.GetFaceSet (&faceSet);
	    if (! m_processBodyTorus->Step (&vertexSet, &edgeSet, &faceSet))
	    {
		m_processBodyTorus = 0;
		cdbg << "End process torus" << endl;
		m_currentBody = (m_currentBody + 1) % 
		    GetSimulation ().GetFoam (0).GetBodies ().size ();
	    }
	}
	widgetGl->CompileUpdate ();
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
    }
}

void MainWindow::updateAllViews3DAverage ()
{
    if (GetSimulationGroup ().GetIndex3DSimulation () == INVALID_INDEX)
	return;
    widgetVtk->RemoveViews ();
    widgetVtk->WidgetBase::ForAllViews (
        boost::bind (&MainWindow::addVtkView, this, _1));
    updateStretch ();
}

void MainWindow::addVtkView (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    const ColorBarModel& scalarColorBarModel = 
        *getScalarColorBarModel (viewNumber);
    const ColorBarModel& velocityColorBarModel = 
        *getColorBarModel (viewNumber, BodyAttribute::VELOCITY);
    QwtDoubleInterval interval;
    if (bodySelector.GetType () == BodySelectorType::PROPERTY_VALUE)
    {
	const vector<QwtDoubleInterval>& v = 
	    static_cast<const PropertyValueBodySelector&> (bodySelector).
	    GetIntervals ();
	interval = v[0];
    }
    else
        interval = scalarColorBarModel.GetInterval ();
    widgetVtk->AddAverageView (viewNumber, scalarColorBarModel, interval,
                               velocityColorBarModel);
}



void MainWindow::createActions ()
{
    m_actionRotateShown = boost::make_shared<QAction> (
	tr("&Rotate Model"), this);
    m_actionRotateShown->setShortcut(QKeySequence (tr ("R")));
    m_actionRotateShown->setStatusTip(tr("Rotate Model"));
    connect(m_actionRotateShown.get (), SIGNAL(triggered()),
	    this, SLOT(RotateShown ()));
    
    m_actionScaleShown = boost::make_shared<QAction> (tr("&Scale"), this);
    m_actionScaleShown->setShortcut(QKeySequence (tr ("Z")));
    m_actionScaleShown->setStatusTip(tr("Scale"));
    connect(m_actionScaleShown.get (), SIGNAL(triggered()),
	    this, SLOT(ScaleShown ()));
    
    m_actionTranslateShown = boost::make_shared<QAction> (
	tr("&Translate"), this);
    m_actionTranslateShown->setShortcut(QKeySequence (tr ("T")));
    m_actionTranslateShown->setStatusTip(tr("Translate"));
    connect(m_actionTranslateShown.get (), SIGNAL(triggered()),
	    this, SLOT(TranslateShown ()));           
    
    m_actionSelectShown = boost::make_shared<QAction> (
	tr("&Select"), this);
    m_actionSelectShown->setShortcut(QKeySequence (tr ("S")));
    m_actionSelectShown->setStatusTip(tr("Select"));
    connect(m_actionSelectShown.get (), SIGNAL(triggered()),
	    this, SLOT(SelectShown ()));
    
    m_actionDeselectShown = boost::make_shared<QAction> (
	tr("&Deselect"), this);
    m_actionDeselectShown->setShortcut (QKeySequence (tr ("D")));
    m_actionDeselectShown->setStatusTip (tr("Deselect"));
    connect(m_actionDeselectShown.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectShown ()));
    
    addAction (widgetGl->GetActionResetTransformAll ().get ());    

    connect (widgetGl->GetActionColorMapEdit ().get (), SIGNAL(triggered()), 
             this, SLOT(ColorMapEdit()));
    connect (widgetVtk->GetActionColorMapEdit ().get (), SIGNAL(triggered()), 
             this, SLOT(ColorMapEdit ()));
    connect (widgetGl->GetActionColorMapClampClear ().get (), 
             SIGNAL (triggered ()), this, SLOT (ColorMapClampClear ()));
    connect (widgetVtk->GetActionColorMapClampClear ().get (), 
             SIGNAL (triggered ()), this, SLOT (ColorMapClampClear ()));
    connect (widgetGl->GetActionOverlayMapEdit ().get (), 
             SIGNAL (triggered ()), this, SLOT(OverlayMapEdit ()));
    connect (widgetVtk->GetActionOverlayMapEdit ().get (), 
             SIGNAL (triggered ()), this, SLOT(OverlayMapEdit ()));
    connect (widgetGl->GetActionOverlayMapClampClear ().get (), 
             SIGNAL (triggered ()), this, SLOT (OverlayMapClampClear ()));
    connect (widgetVtk->GetActionOverlayMapClampClear ().get (), 
             SIGNAL (triggered ()), this, SLOT (OverlayMapClampClear ()));
    connect (widgetGl->GetSignalMapperColorMapCopy ().get (),
	     SIGNAL (mapped (int)), this, SLOT (ColorMapCopy (int)));
    connect (widgetVtk->GetSignalMapperColorMapCopy ().get (),
	     SIGNAL (mapped (int)), this, SLOT (ColorMapCopy (int)));
    connect(widgetGl->GetActionOverlayMapCopyVelocityMagnitude ().get (),
            SIGNAL(triggered()), this, SLOT(OverlayMapCopyVelocityMagnitude ()));
    connect(widgetVtk->GetActionOverlayMapCopyVelocityMagnitude ().get (),
            SIGNAL(triggered()), this, SLOT(OverlayMapCopyVelocityMagnitude ()));

    addAction (sliderTimeSteps->GetActionNextSelectedTimeStep ().get ());
    addAction (sliderTimeSteps->GetActionPreviousSelectedTimeStep ().get ());

    m_actionResetTransformAll = 
        boost::make_shared<QAction> (tr("&All"), this);
    m_actionResetTransformAll->setShortcut(QKeySequence (tr ("Ctrl+R")));
    m_actionResetTransformAll->setStatusTip(tr("Reset transform all"));
    connect (m_actionResetTransformAll.get (), SIGNAL(triggered()),     
             this, SLOT(ResetTransformAll ()));
    addAction (m_actionResetTransformAll.get ());


    addAction (m_actionRotateShown.get ());
    addAction (m_actionTranslateShown.get ());
    addAction (m_actionScaleShown.get ());
    addAction (m_actionSelectShown.get ());
    addAction (m_actionDeselectShown.get ());
}


/**
 * @todo Calculate the correct histogram for the 'domain histogram' image.
 */
MainWindow::HistogramInfo MainWindow::getHistogramInfo (
    ColorBarType::Enum colorBarType, size_t bodyOrFaceScalar) const
{
    const Simulation& simulation = GetSimulation ();
    switch (colorBarType)
    {
    case ColorBarType::STATISTICS_COUNT:
	return createHistogramInfo (widgetGl->GetRangeCount (), 1);
    
    case ColorBarType::PROPERTY:
    {
	const HistogramStatistics& histogramStatistics = 
	    simulation.GetHistogram (bodyOrFaceScalar);
	return HistogramInfo (histogramStatistics.ToQwtIntervalData (), 
			      histogramStatistics.GetMaxCountPerBin ());
    }
    
    case ColorBarType::T1S_KDE:
	return createHistogramInfo (
	    widgetGl->GetRangeT1sKDE (), simulation.GetTopologicalChangeSize ());

    default:
	ThrowException ("Invalid call to getHistogramInfo");
	return createHistogramInfo (widgetGl->GetRangeCount (), 1);
    }
}

MainWindow::HistogramInfo MainWindow::createHistogramInfo (
    pair<float, float> minMax, size_t count) const
{
    QwtArray<QwtDoubleInterval> a (HISTOGRAM_INTERVALS);
    QwtArray<double> d (HISTOGRAM_INTERVALS);
    double intervalSize = (minMax.second - minMax.first) / HISTOGRAM_INTERVALS;
    double begin = minMax.first;
    for (int i = 0; i < a.size (); ++i, begin += intervalSize)
    {
	a[i] = QwtDoubleInterval (begin, begin + intervalSize);
	d[i] = count;
    }
    return HistogramInfo (QwtIntervalData (a, d), count);
}

void MainWindow::setupColorBarModels ()
{
    size_t simulationCount = GetSimulationGroup ().size ();
    m_colorBarModelBodyScalar.resize (simulationCount);
    m_colorBarModelVelocity.resize (simulationCount);
    m_colorBarModelDomainHistogram.resize (simulationCount);
    m_colorBarModelT1sKDE.resize (simulationCount);
    for (size_t simulationIndex = 0; simulationIndex < simulationCount; 
	 ++simulationIndex)
	for (size_t vn = 0; vn < ViewNumber::COUNT; ++vn)
	    setupColorBarModels (simulationIndex, ViewNumber::Enum (vn));
}

void MainWindow::setupColorBarModels (size_t simulationIndex,
				      ViewNumber::Enum viewNumber)
{    
    for (size_t i = 0; i < BodyScalar::COUNT; ++i)
    {
	BodyScalar::Enum property = BodyScalar::FromSizeT (i);
	setupColorBarModelBodyScalar (simulationIndex, viewNumber, property);
    }
    setupColorBarModelVelocityVector (simulationIndex, viewNumber);
    setupColorBarModelDomainHistogram (simulationIndex, viewNumber);
    setupColorBarModelT1sKDE (simulationIndex, viewNumber);
}


void MainWindow::setupViews ()
{
    for (size_t i = 0; i < GetSettings ().GetViewSettingsSize (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
        const ViewSettings& vs = GetViewSettings (viewNumber);
	widgetGl->SetBodyOrFaceScalar (
	    viewNumber, 
	    m_colorBarModelBodyScalar
            [vs.GetSimulationIndex ()][viewNumber][BodyScalar::PRESSURE], 
	    BodyScalar::PRESSURE);
    }
}

void MainWindow::setupColorBarModelT1sKDE (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_colorBarModelT1sKDE[simulationIndex][viewNumber];
    colorBarModel.reset (new ColorBarModel ());
    colorBarModel->SetTitle ("T1s KDE");
    colorBarModel->SetInterval (
	toQwtDoubleInterval (widgetGl->GetRangeT1sKDE (viewNumber)));
    colorBarModel->SetupPalette (
	Palette (PaletteType::SEQUENTIAL, PaletteSequential::BREWER_BLUES9));
}


void MainWindow::setupColorBarModelDomainHistogram (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_colorBarModelDomainHistogram[simulationIndex][viewNumber];
    colorBarModel.reset (new ColorBarModel ());
    colorBarModel->SetTitle ("Count per area");
    colorBarModel->SetInterval (
	toQwtDoubleInterval (widgetGl->GetRangeCount ()));
    colorBarModel->SetupPalette (
	Palette (PaletteType::SEQUENTIAL, PaletteSequential::BLACK_BODY));
}


void MainWindow::setupColorBarModelBodyScalar (size_t simulationIndex,
						 ViewNumber::Enum viewNumber,
						 BodyScalar::Enum property)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_colorBarModelBodyScalar[simulationIndex][viewNumber][property];
    setupColorBarModel (colorBarModel, property, simulationIndex);
}

void MainWindow::setupColorBarModelVelocityVector (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_colorBarModelVelocity[simulationIndex][viewNumber];
    BodyScalar::Enum property = BodyScalar::VELOCITY_MAGNITUDE;
    setupColorBarModel (colorBarModel, property, simulationIndex);
    colorBarModel->SetTitle ("Velocity overlay");
}

void MainWindow::setupColorBarModel (
    boost::shared_ptr<ColorBarModel>& colorBarModel, 
    BodyScalar::Enum property, size_t simulationIndex)
{
    colorBarModel.reset (new ColorBarModel ());
    const Simulation& simulation = GetSimulation (simulationIndex);
    colorBarModel->SetTitle (BodyScalar::ToString (property));
    colorBarModel->SetInterval (simulation.GetRange (property));
    colorBarModel->SetupPalette (
	Palette (PaletteType::DIVERGING, PaletteDiverging::BLUE_RED));
}



boost::shared_ptr<ColorBarModel> MainWindow::getScalarColorBarModel () const
{
    return getScalarColorBarModel (GetViewNumber ());
}


boost::shared_ptr<ColorBarModel> MainWindow::getScalarColorBarModel (
    ViewNumber::Enum viewNumber) const
{
    return getColorBarModel (viewNumber, BodyAttribute::COUNT);
}

boost::shared_ptr<ColorBarModel> MainWindow::getColorBarModel (
    ViewNumber::Enum viewNumber, size_t bodyAttribute) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t simulationIndex = vs.GetSimulationIndex ();
    ViewType::Enum viewType = vs.GetViewType ();
    if (bodyAttribute == BodyAttribute::COUNT)
        bodyAttribute = vs.GetBodyOrFaceScalar ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    return getColorBarModel (simulationIndex, 
			     viewNumber, viewType, bodyAttribute, statisticsType);
}

boost::shared_ptr<ColorBarModel> MainWindow::getColorBarModel (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber,
    ViewType::Enum viewType, size_t property, 
    StatisticsType::Enum statisticsType) const
{
    ColorBarType::Enum colorBarType = ViewSettings::GetColorBarType (
	viewType, property, statisticsType);
    switch (colorBarType)
    {
    case ColorBarType::PROPERTY:
        if (property == BodyAttribute::VELOCITY)
            return m_colorBarModelVelocity[simulationIndex][viewNumber];
        else
            return m_colorBarModelBodyScalar
                [simulationIndex][viewNumber][property];
    case ColorBarType::STATISTICS_COUNT:
	return m_colorBarModelDomainHistogram[simulationIndex][viewNumber];

    case ColorBarType::T1S_KDE:
	return m_colorBarModelT1sKDE[simulationIndex][viewNumber];

    default:
	return boost::shared_ptr<ColorBarModel> ();
    }
}



void MainWindow::clickedPlay (PlayType playType)
{
    bool* playMovie[] = {&m_playForward, &m_playReverse};
    QToolButton* tbPlay[] = {toolButtonPlayForward, toolButtonPlayReverse};
    const char* text[] = {PLAY_FORWARD_TEXT, PLAY_REVERSE_TEXT};
    PlayType playTypeReverse = 
	(playType == PLAY_FORWARD ? PLAY_REVERSE : PLAY_FORWARD);
    if (*playMovie[playType])
    {
	m_timer->stop ();
	tbPlay[playType]->setText (text[playType]);
    }
    else if (*playMovie[playTypeReverse])
    {
	*playMovie[playTypeReverse] = false;
	tbPlay[playTypeReverse]->setText (text[playTypeReverse]);
	tbPlay[playType]->setText (PAUSE_TEXT);
    }
    else
    {
	m_timer->start ();
	tbPlay[playType]->setText (PAUSE_TEXT);
    }
    *playMovie[playType] = ! *playMovie[playType];
    updateButtons ();
}

void MainWindow::currentIndexChangedFaceColor (
    ViewNumber::Enum viewNumber)
{
    int value = static_cast<QComboBox*> (sender ())->currentIndex ();
    const ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
    boost::array<QWidget*, 3> widgetsVisible = {{
	    checkBoxHistogramShown, checkBoxHistogramColorMapped, 
	    checkBoxHistogramAllTimesteps}};
    boost::array<QWidget*, 1> widgetsEnabled = {{
	    radioButtonAverage}};
    size_t simulationIndex = vs.GetSimulationIndex ();
    if (value == FaceScalar::DMP_COLOR) {
	::setVisible (widgetsVisible, false);
	::setEnabled (widgetsEnabled, false);
	Q_EMIT BodyOrFaceScalarChanged (
	    viewNumber,
	    boost::shared_ptr<ColorBarModel> (), value);
    }
    else {
	BodyScalar::Enum property = BodyScalar::FromSizeT (value);
	::setVisible (widgetsVisible, true);
	::setEnabled (widgetsEnabled, true);
	Q_EMIT BodyOrFaceScalarChanged (
	    viewNumber,
	    m_colorBarModelBodyScalar
	    [simulationIndex][viewNumber][property], property);
        widgetHistogram->UpdateColorMapped (
            viewNumber, getScalarColorBarModel (viewNumber));
	widgetHistogram->UpdateData (
            viewNumber, WidgetHistogram::DISCARD_SELECTION, 
            WidgetHistogram::REPLACE_MAX_VALUE);
    }
}

void MainWindow::setStackedWidgetVisualization (ViewType::Enum viewType)
{
    // WARNING: Has to match ViewType::Enum order
    QWidget* pages[] = 
	{
	    pageEmpty,
	    pageEmpty,
	    pageEmpty,
	    pageFacesNormal,

	    pageBubblePaths,
	    pageAverage,
	    pageT1sProbabilityDensity
	};
    stackedWidgetVisualization->setCurrentWidget (pages[viewType]);
}

void MainWindow::setStackedWidgetVelocity (VectorVis::Enum vectorVis)
{
    QWidget* pages[] = 
    {
        stackedWidgetVelocityGlyph,
        stackedWidgetVelocityStreamline,
        stackedWidgetVelocityStreamline
    };
    stackedWidgetVelocity->setCurrentWidget (pages[vectorVis]);
}

void MainWindow::ShowMessageBox (const char* message)
{
    ::ShowMessageBox (this, message);
}


// Slots and methods called by the UI
// ==================================

void MainWindow::SelectionChangedSettings (ViewNumber::Enum viewNumber)
{
    widgetHistogram->UpdateSelection (viewNumber);
    SelectionChangedHistogram (viewNumber);
}

void MainWindow::SelectionChangedHistogram (int vn)
{
    ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (vn);
    ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    const Histogram& histogram = widgetHistogram->GetHistogram (viewNumber);
    BodyScalar::Enum bodyScalar = BodyScalar::FromSizeT (
        vs.GetBodyOrFaceScalar ());

    vector<QwtDoubleInterval> valueIntervals;
    vector<pair<size_t, size_t> > bins;
    histogram.GetSelectedIntervals (&valueIntervals);
    histogram.GetSelectedBins (&bins);
    updateSliderTimeSteps (viewNumber, valueIntervals);
    
    if (histogram.AreAllItemsSelected ())
	vs.SetBodySelector (
	    AllBodySelector::Get (), BodySelectorType::PROPERTY_VALUE);
    else
	vs.SetBodySelector (
	    boost::shared_ptr<PropertyValueBodySelector> (
		new PropertyValueBodySelector (
                    bodyScalar, simulation.Is2D (),
                    valueIntervals, bins)));
    widgetGl->CompileUpdate (viewNumber);
    if (simulation.Is3D ())
    {
	QwtDoubleInterval interval;
	if (valueIntervals.empty ())
	    interval = QwtDoubleInterval (0, -1);
	else
	    interval = valueIntervals[0];
	widgetVtk->UpdateScalarThreshold (interval);
    }
}


void MainWindow::clearStretch (QWidget* widget)
{
    QSizePolicy policy = widget->sizePolicy ();
    policy.setHorizontalStretch (0);
    policy.setVerticalStretch (0);
    widget->setSizePolicy (policy);
}


void MainWindow::updateStretch (QWidget* widget, 
				ViewLayout::Enum layout,
				size_t value)
{
    QSizePolicy policy = widget->sizePolicy ();
    if (layout == ViewLayout::HORIZONTAL)
    {
	policy.setHorizontalStretch (value);
	policy.setVerticalStretch (0);
    }
    else
    {
	policy.setHorizontalStretch (0);
	policy.setVerticalStretch (value);
    }
    widget->setSizePolicy (policy);
}

void MainWindow::updateStretch ()
{
    size_t glCount = GetGlCount ();
    size_t vtkCount = GetVtkCount ();
    if (glCount && vtkCount)
    {
	ViewLayout::Enum layout = GetSettings ().GetViewLayout ();
	updateStretch (widgetGl, layout, glCount);
	updateStretch (widgetVtk, layout, vtkCount);
    }
    else
    {
	clearStretch (widgetGl);
	clearStretch (widgetVtk);
    }
}

void MainWindow::CurrentIndexChangedViewLayout (int index)
{
    GetSettingsPtr ()->SetViewLayout (ViewLayout::Enum (index));
    updateStretch ();
    widgetGl->CompileUpdate ();
    widgetVtk->update ();
    updateAllViews3DAverage ();
    comboBoxWindowLayout->setCurrentIndex (index);
}

void MainWindow::CurrentIndexChangedViewCount (int index)
{
    ViewCount::Enum viewCount = ViewCount::FromSizeT (index + 1);
    GetSettingsPtr ()->SetViewCount (viewCount);
    GetSettingsPtr ()->SetViewNumber (ViewNumber::VIEW0);
    widgetGl->ForAllViews (
	boost::bind (&WidgetGl::SetViewTypeAndCameraDistance, widgetGl, _1));

    boost::array<QWidget*, 2> widgetsViewLayout = 
	{{labelViewLayout, comboBoxViewLayout}};
    if (viewCount == ViewCount::TWO || viewCount == ViewCount::THREE)
	::setVisible (widgetsViewLayout, true);
    else
	::setVisible (widgetsViewLayout, false);
    checkBoxTitleShown->setChecked (viewCount != ViewCount::ONE);    
    GetSettingsPtr ()->SetAverageTimeWindow (
        GetSettings ().GetLinkedTimeSteps ());
    widgetHistogram->UpdateHidden ();
    updateAllViews3DAverage ();
}


void MainWindow::CurrentIndexChangedWindowLayout (int index)
{
    RemoveLayout (widgetContainer);

    // determine new layout
    ViewLayout::Enum windowLayout = ViewLayout::Enum (index);
    QLayout* layout;
    if (windowLayout == ViewLayout::HORIZONTAL)
	layout = new QHBoxLayout ();
    else
	layout = new QVBoxLayout ();
    layout->setSpacing (0);
    layout->setContentsMargins (0, 0, 0, 0);

    // add new layout    
    boost::array<QWidget*, 2> widgets = {{widgetGl, widgetVtk}};
    for (size_t i = 0; i < widgets.size (); ++i)
	layout->addWidget (widgets[i]);
    widgetContainer->setLayout (layout);
    widgetContainer->update ();
}


void MainWindow::ToggledBarLarge (bool large)
{
    GetSettingsPtr ()->SetBarLarge (large);
    widgetGl->update ();
}

void MainWindow::ToggledAxesShown (bool checked)
{
    GetSettingsPtr ()->SetAxesShown (checked);
    widgetGl->CompileUpdateAll ();
    widgetVtk->FromView ();
}

void MainWindow::ToggledScalarShown (bool checked)
{
    GetViewSettings ().SetScalarShown (checked);
    widgetGl->CompileUpdate ();
    widgetVtk->FromView ();
}


void MainWindow::ToggledViewFocusShown (bool checked)
{
    GetSettingsPtr ()->SetViewFocusShown (checked);
    widgetGl->CompileUpdate ();
    widgetVtk->UpdateFocus ();
    widgetHistogram->UpdateFocus ();
}

void MainWindow::initOverlayBarModel ()
{
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	const ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
	size_t simulationIndex = vs.GetSimulationIndex ();
	boost::shared_ptr<ColorBarModel> overlayBarModel = 
	    m_colorBarModelVelocity[simulationIndex][viewNumber];
	Q_EMIT OverlayBarModelChanged (viewNumber, overlayBarModel);
    }    
}


void MainWindow::ToggledVelocityShown (bool shown)
{
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetVelocityShown (shown);
    }
    initOverlayBarModel ();
    radioButtonVelocityGlyph->setEnabled (shown);
    radioButtonVelocityStreamline->setEnabled (shown);
    radioButtonVelocityPathline->setEnabled (shown);
    widgetGl->CompileUpdateAll ();
    widgetVtk->UpdateVelocityAverage ();
}

void MainWindow::ToggledVelocitySameSize (bool checked)
{
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
        ViewSettings& vs = GetViewSettings (viewNumber);
        vs.SetVelocityGlyphSameSize (checked);
    }
    widgetGl->ToggledVelocitySameSize (checked);
    widgetVtk->FromView ();
}


void MainWindow::ToggledHistogramGridShown (bool checked)
{
    widgetHistogram->SetGridShown (checked);
}

void MainWindow::ToggledHistogramShown (bool checked)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetHistogramShown (checked);
    widgetHistogram->UpdateColorMapped (viewNumber, 
                                        getScalarColorBarModel (viewNumber));
    widgetHistogram->UpdateData (
        viewNumber, WidgetHistogram::KEEP_SELECTION, 
        WidgetHistogram::KEEP_MAX_VALUE);    
}


void MainWindow::ToggledHistogramColorMapped (bool checked)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetHistogramOption (HistogramType::COLOR_MAPPED, checked);
    widgetHistogram->UpdateColorMapped (
        viewNumber, getScalarColorBarModel (viewNumber));
    widgetHistogram->UpdateData (
        viewNumber, WidgetHistogram::KEEP_SELECTION, 
        WidgetHistogram::KEEP_MAX_VALUE);
}

void MainWindow::ToggledHistogramAllTimesteps (bool checked)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetHistogramOption (HistogramType::ALL_TIME_STEPS_SHOWN, checked);
    widgetHistogram->UpdateColorMapped (
        viewNumber, getScalarColorBarModel (viewNumber));
    widgetHistogram->UpdateData (
        viewNumber, WidgetHistogram::KEEP_SELECTION, 
        WidgetHistogram::REPLACE_MAX_VALUE);
}


void MainWindow::CellClickedLinkedTimeEvents (int row, int column)
{
    (void) column;
    if (GetSettings ().GetTimeLinkage () == TimeLinkage::LINKED)
        sliderTimeSteps->setValue (GetSettings ().GetLinkedTimeEventTime (row));
    else
        sliderTimeSteps->setValue (
            GetSettings ().GetLinkedTimeEvents (GetViewNumber ())[row]);
}

void MainWindow::ButtonClickedInteractionObject (int index)
{
    GetSettingsPtr ()->SetInteractionObject (InteractionObject::Enum (index));
}


void MainWindow::ButtonClickedPlay ()
{
    clickedPlay (PLAY_FORWARD);
}

void MainWindow::ButtonClickedPlayReverse ()
{
    clickedPlay (PLAY_REVERSE);
}

void MainWindow::ButtonClickedBegin ()
{
    __LOG__ (cdbg << "MainWindow::ButtonClickedBegin" << endl;);
    sliderTimeSteps->setValue (sliderTimeSteps->minimum ());
    updateButtons ();
    
}

void MainWindow::ButtonClickedEnd ()
{
    __LOG__ (cdbg << "MainWindow::ButtonClickedEnd" << endl;);
    sliderTimeSteps->setValue (sliderTimeSteps->maximum ());
    updateButtons ();
}

void MainWindow::TimeoutTimer ()
{
    int value = sliderTimeSteps->value ();
    if (m_playForward)
    {
	if (value < sliderTimeSteps->maximum ())
	    sliderTimeSteps->setValue (value + 1);
	else
	    clickedPlay (PLAY_FORWARD);
    }
    else
    {
	if (value > sliderTimeSteps->minimum ())
	    sliderTimeSteps->setValue (value - 1);
	else
	    clickedPlay (PLAY_REVERSE);
    }
}

void MainWindow::ValueChangedForceSize (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetForceSize (
	IndexExponentToValue (
	    static_cast<QSlider*> (sender ()), ViewSettings::FORCE_SIZE_EXP2));
    widgetGl->CompileUpdate ();
}

void MainWindow::ValueChangedForceLineWidth (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetForceLineWidth (
	IndexExponentToValue (static_cast<QSlider*> (sender ()),
			     ViewSettings::TENSOR_LINE_WIDTH_EXP2));
    widgetGl->CompileUpdate ();
}

void MainWindow::ValueChangedGlyphSeedsCount (int count)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetGlyphSeedsCount (count);
    widgetVtk->FromView ();
}


void MainWindow::ValueChangedContextAlpha (int index)
{
    (void)index;
    GetViewSettings ().SetContextAlpha (
	IndexToValue (static_cast<QSlider*> (sender ()),
                     ViewSettings::ALPHA_RANGE));
    widgetGl->CompileUpdate ();
    widgetVtk->FromView ();
}

void MainWindow::ValueChangedObjectAlpha (int index)
{
    (void)index;
    GetViewSettings ().SetObjectAlpha (
	IndexToValue (static_cast<QSlider*> (sender ()),
                     ViewSettings::ALPHA_RANGE));
    widgetVtk->FromView ();
}

void MainWindow::ValueChangedHistogramHeight (int s)
{
    widgetHistogram->SetHeight (s);
}


void MainWindow::ValueChangedFontSize (int fontSize)
{
    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    defaultFont.setPointSize (fontSize);
    app->setFont (defaultFont);
    widgetHistogram->SetDefaultFont ();
    m_editColorMap->SetDefaultFont ();
}

void MainWindow::ValueChangedT1sKernelSigma (double value)
{
    (void)value;
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	size_t simulationIndex = 
	    GetViewSettings (viewNumber).GetSimulationIndex ();
	m_colorBarModelT1sKDE[simulationIndex][viewNumber]->SetInterval (
	    toQwtDoubleInterval (widgetGl->GetRangeT1sKDE (viewNumber)));
    }
}

void MainWindow::ValueChangedSliderTimeSteps (int timeStep)
{
    __LOG__ (cdbg << "MainWindow::ValueChangedSliderTimeSteps" 
             << timeStep << endl;);
    vector<ViewNumber::Enum> vn = GetSettings ().GetLinkedTimeViewNumbers ();
    boost::array<int, ViewNumber::COUNT> direction;

    GetSettingsPtr ()->SetTime (timeStep, &direction);
    if (GetSettings ().GetTimeLinkage () == TimeLinkage::LINKED)
    {
        GetSettingsPtr ()->UpdateAverageTimeWindow ();
        timeViewToUI (GetViewNumber ());
    }
    for (size_t i = 0; i < vn.size (); ++i)
    {
        ViewNumber::Enum viewNumber = vn[i];
        ViewSettings& vs = GetViewSettings (viewNumber);
        const Simulation& simulation = GetSimulation (viewNumber);
        ViewType::Enum viewType = vs.GetViewType ();
        if (vs.GetTime () < GetSettings ().GetLinkedTime ())
            continue;
        widgetHistogram->UpdateColorMapped (
            viewNumber, getScalarColorBarModel (viewNumber));
        widgetHistogram->UpdateData (viewNumber,
                                     WidgetHistogram::KEEP_SELECTION, 
                                     WidgetHistogram::KEEP_MAX_VALUE);
        if (viewType == ViewType::AVERAGE || viewType == ViewType::T1S_KDE)
        {
            if (simulation.Is3D ())
                widgetVtk->UpdateAverage (viewNumber, direction[viewNumber]);
            else
                widgetGl->UpdateAverage (viewNumber, direction[viewNumber]);
        }
        widgetGl->CompileUpdate (viewNumber);

        if (m_debugTranslatedBody)
        {
            const Foam& foam = GetSimulation ().GetFoam (0);
            m_currentTranslatedBody = 
                const_cast<Foam&> (foam).GetBodies ().begin ();
        }
    }
    widgetVtk->update ();
    updateButtons ();
}

void MainWindow::ValueChangedAverageTimeWindow (int timeSteps)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    if (GetSettings ().GetTimeLinkage () == TimeLinkage::INDEPENDENT)
        GetViewSettings (viewNumber).SetTimeWindow (timeSteps);
    else
        GetSettingsPtr ()->SetAverageTimeWindow (timeSteps);
    timeViewToUI (viewNumber);
}


void MainWindow::ButtonClickedVelocityVis (int vv)
{
    VectorVis::Enum velocityVis = VectorVis::Enum (vv);
    setStackedWidgetVelocity (velocityVis); 
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
        ViewNumber::Enum viewNumber = vn[i];
        ViewSettings& vs = GetViewSettings (viewNumber);
        vs.SetVelocityVis (velocityVis);
        if (vs.GetVelocityVis () == VectorVis::STREAMLINE)
            widgetGl->CacheCalculateStreamline (viewNumber);
    }
    widgetGl->update ();
}

void MainWindow::ButtonClickedForce (int t)
{
    ForceType::Enum type =ForceType::FromSizeT (t);
    ViewSettings& vs = GetViewSettings ();
    if (type == ForceType::DIFFERENCE &&
        (! vs.IsAverageAround () ||
         vs.GetAverageAroundSecondBodyId () == INVALID_INDEX))
    {
	ShowMessageBox ("This feature works only when "
                        "averaging around two objects.");
	SetCheckedNoSignals (checkBoxForceDifference, false, true);
	return;
    }
    vs.SetForceShown (type, ! vs.IsForceShown (type));
    widgetGl->CompileUpdate ();
    widgetVtk->UpdateForceAverage ();
}

void MainWindow::ButtonClickedTorque (int t)
{
    ForceType::Enum type =ForceType::FromSizeT (t);
    ViewSettings& vs = GetViewSettings ();
    vs.SetTorqueShown (type, vs.IsTorqueShown (type));
    widgetGl->CompileUpdate ();
}

void MainWindow::ButtonClickedViewType (int vt)
{
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    ViewType::Enum viewType = ViewType::Enum(vt);
    ViewType::Enum oldViewType = 
        GetSettingsPtr ()->SetTwoHalvesViewType (viewType);
    widgetTimeWindow->setVisible (
        viewType == ViewType::AVERAGE || viewType == ViewType::T1S_KDE);
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	const Simulation& simulation = GetSimulation (viewNumber);

	size_t simulationIndex = vs.GetSimulationIndex ();
	ViewType::Enum oldViewType = vs.GetViewType ();
	size_t property = vs.GetBodyOrFaceScalar ();
	StatisticsType::Enum statisticsType = vs.GetStatisticsType ();

	setStackedWidgetVisualization (viewType);
	Q_EMIT ColorBarModelChanged (
	    viewNumber, getColorBarModel (
                simulationIndex, viewNumber, viewType, 
                property, statisticsType));

	switch (viewType)
	{
	case ViewType::FACES:
	    break;

	case ViewType::AVERAGE:
	    labelAverageColor->setText (
		BodyScalar::ToString (BodyScalar::FromSizeT (property)));
	    break;

	case ViewType::CENTER_PATHS:
	    labelBubblePathsColor->setText (
		BodyScalar::ToString (BodyScalar::FromSizeT (property)));
	    break;

	case ViewType::T1S_KDE:
	    sliderTimeSteps->setMaximum (
                simulation.GetTopologicalChangeTimeSteps () - 1);
	    break;

	default:
	    break;
	}
	if (oldViewType == ViewType::T1S_KDE)
	    sliderTimeSteps->setMaximum (simulation.GetTimeSteps () - 1);
    }
    widgetGl->ButtonClickedViewType (oldViewType);
    updateAllViews3DAverage ();
}

void MainWindow::ButtonClickedTimeLinkage (int id)
{
    TimeLinkage::Enum timeLinkage = TimeLinkage::Enum (id);
    if (timeLinkage == TimeLinkage::LINKED && ! 
        GetSettings ().HasEqualNumberOfEvents ())
    {
        ShowMessageBox ("You have to have an equal number of events "
                        "in boths views");
        radioButtonTimeIndependent->setChecked (true);
        return;
    }
    GetSettingsPtr ()->SetTimeLinkage (timeLinkage);
    widgetGl->CompileUpdateAll ();
}


void MainWindow::CurrentIndexChangedSimulation (int simulationIndex)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceScalar ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    Q_EMIT ColorBarModelChanged (
	viewNumber,
	getColorBarModel (
            simulationIndex, viewNumber, viewType, property, statisticsType));
    widgetGl->CurrentIndexChangedSimulation (simulationIndex);
    ViewToUI (viewNumber);
}

void MainWindow::CurrentIndexChangedSelectedLight (int i)
{
    const ViewSettings& vs = GetViewSettings ();
    lightViewToUI (vs, LightNumber::Enum (i));
}

void MainWindow::CurrentIndexChangedFaceColor (int value)
{
    (void)value;
    GetSettingsPtr ()->SetOneOrTwoViews (
        this, &MainWindow::currentIndexChangedFaceColor);
}

void MainWindow::CurrentIndexChangedStatisticsType (int value)
{
    boost::array<QWidget*, 2> widgetsAverageTimeWindow = 
	{{ spinBoxAverageTimeWindow, labelAverageTimeWindow}};
    switch (value)
    {
    case StatisticsType::AVERAGE:
	connectColorBarHistogram (true);
	::setVisible (widgetsAverageTimeWindow, true);
	break;
    case StatisticsType::MIN:
    case StatisticsType::MAX:
	connectColorBarHistogram (true);
	::setVisible (widgetsAverageTimeWindow, false);
	break;
    case StatisticsType::COUNT:
	connectColorBarHistogram (false);
	::setVisible (widgetsAverageTimeWindow, false);
	break;
    }
    Q_EMIT ColorBarModelChanged (GetViewNumber (), 
				 getScalarColorBarModel ());
}

void MainWindow::ToggledBubblePathsLineUsed (bool checked)
{
    checkBoxBubblePathsTubeUsed->setEnabled (! checked);        
}


void MainWindow::ToggledTwoHalvesView (bool reflectedHalfView)
{
    if (reflectedHalfView &&
	(GetViewCount () != ViewCount::TWO || 
	 GetSettings ().GetViewLayout () != ViewLayout::VERTICAL))
    {
	ShowMessageBox ("This feature works only with two views "
                        "in vertical layout");
	SetCheckedNoSignals (checkBoxTwoHalvesView, false, true);
	return;
    }
    checkBoxTitleShown->setChecked (false);
    GetSettingsPtr ()->SetTwoHalvesView (
        reflectedHalfView, GetSimulation (), 
        widgetGl->width (), widgetGl->height ());
    widgetGl->CompileUpdate ();
}

void MainWindow::ToggledTitleShown (bool checked)
{
    GetSettingsPtr ()->SetTitleShown (checked);
    widgetGl->update ();
    widgetVtk->UpdateAverage3dTitle ();
}

void MainWindow::updateSliderTimeSteps (
    ViewNumber::Enum viewNumber,
    const vector<QwtDoubleInterval>& valueIntervals)
{
    vector<bool> timeStepSelection;
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    BodyScalar::Enum bodyScalar = BodyScalar::FromSizeT (
        vs.GetBodyOrFaceScalar ());
    simulation.GetTimeStepSelection (
	bodyScalar, valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);
    
}

void MainWindow::ColorMapClampClear ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    boost::shared_ptr<ColorBarModel> colorBarModel = vs.GetColorBarModel ();
    colorBarModel->SetClampClear ();
    Q_EMIT ColorBarModelChanged (viewNumber, colorBarModel);
}

void MainWindow::OverlayMapClampClear ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    boost::shared_ptr<ColorBarModel> colorBarModel = vs.GetOverlayBarModel ();
    colorBarModel->SetClampClear ();
    Q_EMIT OverlayBarModelChanged (viewNumber, colorBarModel);
}

void MainWindow::ColorMapCopy (int other)
{
    ViewSettings& otherVs = GetViewSettings (ViewNumber::Enum (other));
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.ColorMapCopy (otherVs);
    Q_EMIT ColorBarModelChanged (viewNumber, vs.GetColorBarModel ());
}

void MainWindow::OverlayMapCopyVelocityMagnitude ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t simulationIndex = vs.GetSimulationIndex ();
    boost::shared_ptr<ColorBarModel> colorBarModel = 
	m_colorBarModelVelocity[simulationIndex][viewNumber];
    vs.OverlayMapCopyVelocityMagnitude ();
    Q_EMIT OverlayBarModelChanged (GetViewNumber (), colorBarModel);
}


void MainWindow::OverlayMapEdit ()
{
    HistogramInfo p = getHistogramInfo (
	ColorBarType::PROPERTY, BodyScalar::VELOCITY_MAGNITUDE);
    ViewNumber::Enum viewNumber = GetViewNumber ();
    size_t simulationIndex = 
	GetViewSettings (viewNumber).GetSimulationIndex ();
    boost::shared_ptr<ColorBarModel> colorBarModel = 
	m_colorBarModelVelocity[simulationIndex][viewNumber];
    m_editColorMap->SetData (p.first, p.second, 
			     *colorBarModel,
			     checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*colorBarModel = m_editColorMap->GetColorBarModel ();
	Q_EMIT OverlayBarModelChanged (GetViewNumber (), colorBarModel);
    }
}


void MainWindow::ColorMapEdit ()
{
    HistogramInfo p = getHistogramInfo (
	GetSettings ().GetColorBarType (), widgetGl->GetBodyOrFaceScalar ());
    m_editColorMap->SetData (
	p.first, p.second, *getScalarColorBarModel (),
	checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*getScalarColorBarModel () = m_editColorMap->GetColorBarModel ();
	Q_EMIT ColorBarModelChanged (GetViewNumber (),
				     getScalarColorBarModel ());
    }
}


void MainWindow::SetHistogramColorBarModel (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<ColorBarModel> colorBarModel)
{
    if (colorBarModel)
	widgetHistogram->GetHistogram (
	    viewNumber).SetColorTransferFunction (colorBarModel);
}

void MainWindow::CurrentIndexChangedInteractionMode (int index)
{
    GetSettingsPtr ()->SetInteractionMode (InteractionMode::Enum(index));
    InteractionMode::Enum im = InteractionMode::Enum(index);
    radioButtonInteractionLight->setDisabled (true);
    radioButtonInteractionContext->setDisabled (true);
    radioButtonInteractionSeeds->setDisabled (true);
    switch (im)
    {
    case InteractionMode::ROTATE:
	radioButtonInteractionLight->setEnabled (true);
	comboBoxInteractionMode->setToolTip (
	    "Rotate around the X and Y axes<br>"
            "2D only:<br>"
	    "press Ctrl to rotate around the X axis<br>"
	    "press Shift to rotate around the Y axis<br>"
	    "press Alt (or Ctrl+Shift) to rotate around the Z axis.");
	break;

    case InteractionMode::SCALE:
	radioButtonInteractionContext->setEnabled (true);
	radioButtonInteractionSeeds->setEnabled (true);
	comboBoxInteractionMode->setToolTip ("");
	break;
	
    case InteractionMode::TRANSLATE:
	radioButtonInteractionLight->setEnabled (true);
	radioButtonInteractionSeeds->setEnabled (true);
	comboBoxInteractionMode->setToolTip (
	    "Translate in the X and Y plane<br>"
            "2D only:<br>"
	    "press Ctrl to translate along the X axis<br>"
	    "press Shift to translate along the Y axis<br>"
	    "press Alt (or Ctrl+Shift) to translate along the Z axis.");
	break;
    
    default:
	comboBoxInteractionMode->setToolTip ("");
	break;
    }
    widgetHistogram->CurrentIndexChangedInteractionMode (index);
}


void MainWindow::lightViewToUI (
    const ViewSettings& vs, LightNumber::Enum i)
{
    checkBoxLightEnabled->setChecked (vs.IsLightEnabled (i));
    checkBoxLightPositionShown->setChecked (vs.IsLightPositionShown (i));
    checkBoxDirectionalLightEnabled->setChecked (
	vs.IsDirectionalLightEnabled (i));
    horizontalSliderLightAmbientRed->setValue (
	floor (vs.GetLight (i, LightType::AMBIENT)[0] * 
	       horizontalSliderLightAmbientRed->maximum () + 0.5));
    horizontalSliderLightAmbientGreen->setValue (
	floor (vs.GetLight (i, LightType::AMBIENT)[1] * 
	       horizontalSliderLightAmbientGreen->maximum () + 0.5));
    horizontalSliderLightAmbientBlue->setValue (
	floor (vs.GetLight (i, LightType::AMBIENT)[2] * 
	       horizontalSliderLightAmbientBlue->maximum () + 0.5));
    horizontalSliderLightDiffuseRed->setValue (
	floor (vs.GetLight (i, LightType::DIFFUSE)[0] * 
	       horizontalSliderLightDiffuseRed->maximum () + 0.5));
    horizontalSliderLightDiffuseGreen->setValue (
	floor (vs.GetLight (i, LightType::DIFFUSE)[1] * 
	       horizontalSliderLightDiffuseGreen->maximum () + 0.5));
    horizontalSliderLightDiffuseBlue->setValue (
	floor (vs.GetLight (i, LightType::DIFFUSE)[2] * 
	       horizontalSliderLightDiffuseBlue->maximum () + 0.5));
    horizontalSliderLightSpecularRed->setValue (
	floor (vs.GetLight (
		   i, LightType::SPECULAR)[0] * 
	       horizontalSliderLightSpecularRed->maximum () + 0.5));
    horizontalSliderLightSpecularGreen->setValue (
	floor (vs.GetLight (
		   i, LightType::SPECULAR)[1] * 
	       horizontalSliderLightSpecularGreen->maximum () + 0.5));
    horizontalSliderLightSpecularBlue->setValue (
	floor (vs.GetLight (
		   i, LightType::SPECULAR)[2] * 
	    horizontalSliderLightSpecularBlue->maximum () + 0.5));
}


void MainWindow::linkedTimeEventsViewToUI (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ostringstream ostr;
    ostr << vs.GetTimeSteps ();
    labelTimeSteps->setText (ostr.str ().c_str ());
    const vector<size_t>& events = vs.GetLinkedTimeEvents ();
    tableWidgetEvents->setRowCount (events.size ());
    for (size_t eventIndex = 0; eventIndex < events.size (); ++eventIndex)
    {
        size_t begin = eventIndex == 0 ? 0 : events[eventIndex - 1];
        ostr.str ("");
        ostr << begin;
        QTableWidgetItem* item = new QTableWidgetItem (ostr.str ().c_str ());
        tableWidgetEvents->setItem (eventIndex, 0, item);

        size_t end = events[eventIndex];
        ostr.str ("");
        ostr << end;
        item = new QTableWidgetItem (ostr.str ().c_str ());
        tableWidgetEvents->setItem (eventIndex, 1, item);

        string s;
        if (GetSettings ().HasEqualNumberOfEvents ())
        {
            float stretch = GetSettings ().GetLinkedTimeStretch (
                viewNumber, eventIndex);
            ostr.str ("");
            ostr << setprecision (3) << stretch;
            s = ostr.str ();
        }
        item = new QTableWidgetItem (s.c_str ());
        tableWidgetEvents->setItem (eventIndex, 2, item);
    }
}

void MainWindow::deformationViewToUI ()
{
    const ViewSettings& vs = GetViewSettings ();
    const Simulation& simulation = GetSimulation ();
    SetCheckedNoSignals (checkBoxDeformationShown, 
			 vs.IsDeformationShown ());
    bool gridShown = false;
    bool gridCellCenterShown = false;
    if (simulation.Is2D ())
    {
	AttributeAverages2D& va = widgetGl->GetAttributeAverages2D ();
	gridShown = va.GetDeformationAverage ().IsGridShown ();
	gridCellCenterShown = 
	    va.GetDeformationAverage ().IsGridCellCenterShown ();
    }

    SetCheckedNoSignals (checkBoxDeformationGridShown, gridShown);
    SetCheckedNoSignals (checkBoxDeformationGridCellCenterShown, 
			 gridCellCenterShown);
    SetValueNoSignals (
	horizontalSliderDeformationSize, 
	ValueToExponentIndex (horizontalSliderDeformationSize, 
		    WidgetGl::TENSOR_SIZE_EXP2, vs.GetDeformationSize ()));
    SetValueNoSignals (
	horizontalSliderDeformationLineWidth, 
	ValueToExponentIndex (horizontalSliderDeformationLineWidth,
		     ViewSettings::TENSOR_LINE_WIDTH_EXP2,
		     vs.GetDeformationLineWidth ()));
}

void MainWindow::velocityViewToUI ()
{
    const ViewSettings& vs = GetViewSettings ();
    const Simulation& simulation = GetSimulation ();
    bool gridShown = false;
    bool clampingShown = false;
    bool gridCellCenterShown = false;
    bool sameSize = false;
    bool colorMapped = false;
    if (simulation.Is2D ())
    {
	const VectorAverage& va = 
	    widgetGl->GetAttributeAverages2D ().GetVelocityAverage ();
	gridShown = va.IsGridShown ();
	clampingShown = va.IsClampingShown ();
	gridCellCenterShown = va.IsGridCellCenterShown ();
	sameSize = va.IsSameSize ();
	colorMapped = va.IsColorMapped ();
    }

    SetCheckedNoSignals (checkBoxVelocityShown, vs.IsVelocityShown ());
    SetCheckedNoSignals (buttonGroupVelocityVis, vs.GetVelocityVis (), true);
    // glyphs
    SetCheckedNoSignals (checkBoxVelocityGlyphGridShown, gridShown);
    SetCheckedNoSignals (checkBoxVelocityGlyphClampingShown, clampingShown);
    SetCheckedNoSignals (checkBoxVelocityGlyphGridCellCenterShown, 
			 gridCellCenterShown);
    SetCheckedNoSignals (checkBoxVelocityGlyphSameSize, sameSize);
    SetCheckedNoSignals (checkBoxVelocityColorMapped, colorMapped);
    SetValueNoSignals (
	horizontalSliderVelocityGlyphLineWidth, 
	ValueToExponentIndex (horizontalSliderVelocityGlyphLineWidth,
			     ViewSettings::TENSOR_LINE_WIDTH_EXP2,
			     vs.GetVelocityLineWidth ()));
    SetValueNoSignals (spinBoxGlyphSeedsCount, vs.GetGlyphSeedsCount ());
    // streamlines
    const int ratio = 5;
    SetValueAndMaxNoSignals (doubleSpinBoxStreamlineLength,
			     vs.GetStreamlineLength (), 
                             ratio * ViewSettings::STREAMLINE_LENGTH);
    SetValueAndMaxNoSignals (doubleSpinBoxStreamlineStepLength,
			     vs.GetStreamlineStepLength (),
                             ratio * ViewSettings::STREAMLINE_STEP_LENGTH);
}

void MainWindow::forceViewToUI ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    // force
    SetCheckedNoSignals (
	checkBoxForceNetwork, vs.IsForceShown (ForceType::NETWORK), 
        simulation.IsForceAvailable ());
    SetCheckedNoSignals (
	checkBoxForcePressure, 
	vs.IsForceShown (ForceType::PRESSURE), simulation.IsForceAvailable ());
    SetCheckedNoSignals (
	checkBoxForceResult, 
        vs.IsForceShown (ForceType::RESULT), simulation.IsForceAvailable ());
    SetCheckedNoSignals (
	checkBoxForceDifference, 
        vs.IsForceShown (ForceType::DIFFERENCE), simulation.IsForceAvailable ());
    // torque
    SetCheckedNoSignals (
	checkBoxTorqueNetwork, 
        vs.IsTorqueShown (ForceType::NETWORK), simulation.IsTorqueAvailable ());
    SetCheckedNoSignals (
	checkBoxTorquePressure, 
        vs.IsTorqueShown (ForceType::PRESSURE), simulation.IsTorqueAvailable ());
    SetCheckedNoSignals (
	checkBoxTorqueResult, 
        vs.IsTorqueShown (ForceType::RESULT), simulation.IsTorqueAvailable ());
    SetValueNoSignals (
	horizontalSliderTorqueDistance, ValueToExponentIndex (
            horizontalSliderTorqueDistance,
            ViewSettings::FORCE_SIZE_EXP2, vs.GetTorqueDistance ()));
    // size and width
    SetValueNoSignals (
	horizontalSliderForceSize, ValueToExponentIndex (
            horizontalSliderForceSize,
            ViewSettings::FORCE_SIZE_EXP2, vs.GetForceSize ()));
    SetValueNoSignals (
	horizontalSliderForceLineWidth, ValueToExponentIndex (
            horizontalSliderForceLineWidth,
            ViewSettings::TENSOR_LINE_WIDTH_EXP2, vs.GetForceLineWidth ()));
}

void MainWindow::t1sKDEViewToUI (ViewNumber::Enum viewNumber)
{
    const Simulation& simulation = GetSimulation (viewNumber);
    if (simulation.Is2D ())
    {
        bool kernelTextureShown = false;
	const T1sKDE& kde = 
            widgetGl->GetAttributeAverages2D (
                viewNumber).GetTopologicalChangeKDE ();
	kernelTextureShown = kde.IsKernelTextureShown ();
        SetCheckedNoSignals (checkBoxTextureShown, kernelTextureShown);
        SetValueNoSignals (
            doubleSpinBoxKernelSigma, kde.GetKernelSigmaInBubbleDiameters ());
        const Simulation& simulation = GetSimulation (viewNumber);
        radioButtonT1sKDE->setEnabled (
            simulation.IsTopologicalChangeAvailable ());
    }
}

void MainWindow::bubblePathsViewToUI ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    const ViewSettings& vs = GetViewSettings ();
    int property = vs.GetBodyOrFaceScalar ();
    const Simulation& simulation = GetSimulation ();

    labelBubblePathsColor->setText (FaceScalar::ToString (property));
    SetCheckedNoSignals (checkBoxBubblePathsPartialPathHidden, 
                         vs.IsPartialPathHidden ());    
    SetValueNoSignals (horizontalSliderBubblePathsTimeDisplacement,
                       widgetGl->TimeDisplacementToSlider (
                           vs.GetTimeDisplacement (), 
                           *horizontalSliderBubblePathsTimeDisplacement,
                           simulation));
    SetValueAndMaxNoSignals (spinBoxBubblePathsTimeBegin,
			     vs.GetBubblePathsTimeBegin (), 
			     widgetGl->GetTimeSteps (viewNumber));
    SetValueAndMaxNoSignals (spinBoxBubblePathsTimeEnd,
			     vs.GetBubblePathsTimeEnd (), 
			     widgetGl->GetTimeSteps (viewNumber));
}


void MainWindow::timeViewToUI (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    size_t scalarAverageTimeWindow = 0;
    if (GetSettings ().GetTimeLinkage () == TimeLinkage::INDEPENDENT)
        scalarAverageTimeWindow = vs.GetTimeWindow ();
    else
    {
        ostringstream ostr;
        scalarAverageTimeWindow = GetSettings ().GetLinkedTimeWindow ();
        ostr << vs.GetTimeWindow ();
        labelViewTimeWindow->setText (ostr.str ().c_str ());
    }
    size_t steps;
    if (GetSettings ().GetTimeLinkage () == TimeLinkage::INDEPENDENT)
    {
        steps = GetSettings ().GetTimeSteps (viewNumber);
        if (sliderTimeSteps->maximum () == static_cast<int> (steps) - 1)
            sliderTimeSteps->SetValueNoSignals (vs.GetTime ());
        else
            sliderTimeSteps->SetValueAndMaxNoSignals (vs.GetTime (), steps - 1);
    }
    else
    {
	steps = GetSettings ().GetLinkedTimeSteps ();
        if (sliderTimeSteps->maximum () == static_cast<int> (steps) - 1)
            sliderTimeSteps->SetValueNoSignals (
                GetSettings ().GetLinkedTime ());
        else
            sliderTimeSteps->SetValueAndMaxNoSignals (
                GetSettings ().GetLinkedTime (), steps - 1);
    }
    ViewType::Enum viewType = vs.GetViewType ();
    widgetTimeWindow->setVisible (
        viewType == ViewType::AVERAGE || 
        viewType == ViewType::T1S_KDE);
    SetValueAndMaxNoSignals (spinBoxAverageTimeWindow,
			     scalarAverageTimeWindow, steps);
}

void MainWindow::settingsViewToUI (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    SetValueNoSignals (
        horizontalSliderContextAlpha, 
        ValueToIndex (horizontalSliderContextAlpha, 
                      ViewSettings::ALPHA_RANGE, vs.GetContextAlpha ()));
    SetValueNoSignals (
        horizontalSliderObjectAlpha, 
        ValueToIndex (horizontalSliderObjectAlpha, 
                      ViewSettings::ALPHA_RANGE, vs.GetObjectAlpha ()));
}

void MainWindow::ViewToUI (ViewNumber::Enum prevViewNumber)
{
    (void)prevViewNumber;
    ViewNumber::Enum viewNumber = GetViewNumber ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    int property = vs.GetBodyOrFaceScalar ();
    size_t simulationIndex = vs.GetSimulationIndex ();
    ViewType::Enum viewType = vs.GetViewType ();

    widgetHistogram->UpdateFocus ();
    widgetVtk->UpdateFocus ();
    widgetGl->update ();

    SetCheckedNoSignals (buttonGroupViewType, viewType, true);    
    setStackedWidgetVisualization (viewType);
    setStackedWidgetVelocity (vs.GetVelocityVis ());
    SetCheckedNoSignals (checkBoxHistogramShown, vs.IsHistogramShown ());
    SetCheckedNoSignals (
	checkBoxHistogramColorMapped, 
	vs.HasHistogramOption(HistogramType::COLOR_MAPPED));
    SetCheckedNoSignals (
	checkBoxHistogramAllTimesteps,
	vs.HasHistogramOption(HistogramType::ALL_TIME_STEPS_SHOWN));
    SetCheckedNoSignals (checkBoxDomainClipped, vs.DomainClipped ());

    SetCurrentIndexNoSignals (comboBoxColor, property);
    SetCurrentIndexNoSignals (comboBoxSimulation, simulationIndex);
    SetCurrentIndexNoSignals (comboBoxStatisticsType, vs.GetStatisticsType ());
    SetCurrentIndexNoSignals (comboBoxAxisOrder, vs.GetAxisOrder ());

    SetCheckedNoSignals (checkBoxSelectionContextShown, 
			 vs.IsSelectionContextShown ());
    SetCheckedNoSignals (checkBoxT1sShiftLower, vs.T1sShiftLower ());
    SetValueNoSignals (horizontalSliderAngleOfView, vs.GetAngleOfView ());
    labelAverageColor->setText (FaceScalar::ToString (property));

    settingsViewToUI (viewNumber);
    deformationViewToUI ();
    velocityViewToUI ();
    forceViewToUI ();
    t1sKDEViewToUI (viewNumber);
    bubblePathsViewToUI ();
    timeViewToUI (viewNumber);
    linkedTimeEventsViewToUI (viewNumber);
    lightViewToUI (vs, vs.GetSelectedLight ());
    updateButtons ();

    const AttributeHistogram& histogram = 
        widgetHistogram->GetHistogram (viewNumber);
    if (histogram.GetYAxisMaxValue () == 0)
        sliderTimeSteps->SetFullRange ();
    else
    {
        vector<QwtDoubleInterval> valueIntervals;
        widgetHistogram->GetHistogram (
            viewNumber).GetSelectedIntervals (&valueIntervals);
        updateSliderTimeSteps (viewNumber, valueIntervals);
    }
}
