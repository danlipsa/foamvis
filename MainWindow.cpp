/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "Application.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "ScalarAverage.h"
#include "Debug.h"
#include "EditColorMap.h"
#include "Foam.h"
#include "Simulation.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ProcessBodyTorus.h"
#include "SystemDifferences.h"
#include "T1sPDE.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "OpenGLUtils.h"
#include "VectorAverage.h"
#include "ViewSettings.h"


// Private Functions
// ======================================================================

int checkedId (const QButtonGroup* group)
{
    return group->id (group->checkedButton ());
}

HistogramType::Enum histogramType (const QButtonGroup* group)
{
    return HistogramType::Enum (checkedId (group));
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

MainWindow::MainWindow (SimulationGroup& simulationGroup) : 
    m_timer (new QTimer(this)),
    m_processBodyTorus (0), 
    m_debugTranslatedBody (false),
    m_currentBody (0),
    m_histogramType (HistogramType::NONE),
    m_histogramViewNumber (ViewNumber::COUNT),
    m_editColorMap (new EditColorMap (this)),
    m_playForward (false),
    m_playReverse (false)
{
    // for anti-aliased lines
    QGLFormat format = QGLFormat::defaultFormat ();
    format.setSampleBuffers (true);
    format.setAlpha (true);
    QGLFormat::setDefaultFormat(format);
    
    setupUi (this);
    connectSignals ();
    setupButtonGroups ();

    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    spinBoxFontSize->setValue (defaultFont.pointSize ());
    spinBoxHistogramHeight->setMaximum (500);
    spinBoxHistogramHeight->setValue (widgetHistogram->sizeHint ().height ());

    CurrentIndexChangedViewCount (ViewCount::ONE);
    widgetGl->SetStatus (labelStatusBar);
    widgetGl->SetSimulationGroup (&simulationGroup);
    setupColorBarModels ();
    setupViews ();
    initComboBoxSimulation (simulationGroup);
    configureInterface ();
    setupHistogram ();
    createActions ();
    setTabOrder (radioButtonCenterPath, sliderTimeSteps);
    string title ("FoamVis");
    if (simulationGroup.size () == 1)
    {
	Simulation& simulation = simulationGroup.GetSimulation (0);
	title += " - ";
	title += simulation.GetName ();
    }
    setWindowTitle (QString (title.c_str ()));
    widgetHistogram->setHidden (true);
    // fire as soon as all events have been processed
    m_timer->setInterval (0);
    //initTranslatedBody ();
    configureInterfaceDataDependent (simulationGroup);    
}

void MainWindow::configureInterface ()
{
    tabWidget->setCurrentWidget (timeStep);
    comboBoxWindowSize->setCurrentIndex (WindowSize::GL_720x480);
    horizontalSliderDeformationSize->setValue (49);
    horizontalSliderVelocitySize->setValue (49);
    horizontalSliderForceSize->setValue (49);
    comboBoxColor->setCurrentIndex (BodyProperty::PRESSURE);
}


void MainWindow::configureInterfaceDataDependent (
    const SimulationGroup& simulationGroup)
{
    const Simulation& simulation = simulationGroup.GetSimulation (0);
    setupSliderData (simulation);
    if (simulation.T1sAvailable ())
    {
	checkBoxT1sShown->setEnabled (true);	
	checkBoxT1sShown->setChecked (true);
	radioButtonT1sPDE->setEnabled (true);
    }
    if (! simulation.IsTorus ())
    {
	checkBoxTorusOriginalDomain->setDisabled (true);
	checkBoxTorusOriginalDomainWrapInside->setDisabled (true);
	radioButtonEdgesTorus->setDisabled (true);
	radioButtonFaceEdgesTorus->setDisabled (true);
    }
    else if (simulation.Is2D () && simulationGroup.size () == 2)
    {
	G3D::AABox box = simulation.GetBoundingBoxTorus ();
	G3D::Vector3 extent = box.extent ();
	if (extent.x > extent.y)
	    comboBoxViewLayout->setCurrentIndex (ViewLayout::VERTICAL);
    }
    if (simulation.Is2D ())
    {
	comboBoxInteractionMode->setCurrentIndex (InteractionMode::SCALE);	
    }
    else
    {
	comboBoxAxesOrder->setCurrentIndex (AxesOrder::THREE_D);
    }
    size_t viewCount = min (simulationGroup.size (),
			    ViewCount::GetCount (ViewCount::MAX));
    if (simulationGroup.size () > 1)
	comboBoxViewCount->setCurrentIndex (ViewCount::FromSizeT (viewCount));
    for (size_t i = 1; i < viewCount; ++i)
    {
	widgetGl->SetViewNumber (ViewNumber::Enum (i));
	comboBoxSimulation->setCurrentIndex (i);
    }
    widgetGl->SetViewNumber (ViewNumber::VIEW0);
}


void MainWindow::initComboBoxSimulation (SimulationGroup& simulationGroup)
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
    Foam& foam = const_cast<Foam&>
	(widgetGl->GetSimulation ().GetFoam (0));
    m_currentTranslatedBody = foam.GetBodies ().begin ();
}


void MainWindow::connectSignals ()
{
    
    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));
    
    connect (widgetGl, SIGNAL (PaintedGL ()),
	     widgetDisplay, SLOT (SaveFrame ()));
    
    
    // BodyOrFacePropertyChanged: 
    // from MainWindow to GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (BodyOrFacePropertyChanged (
		    boost::shared_ptr<ColorBarModel>, size_t)),
	widgetGl, 
	SLOT (SetBodyOrFaceProperty (
		  boost::shared_ptr<ColorBarModel>, size_t)));
    
    // ColorBarModelChanged:
    // from MainWindow to ColorBar, GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (SetColorBarModel (boost::shared_ptr<ColorBarModel>)));
    
    
    // ColorBarModelChanged:
    // from GLWidget to GLWidget and MainWindow and AttributeHistogram
    connect (
	widgetGl, 
	SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (SetColorBarModel (boost::shared_ptr<ColorBarModel>)));
    
    connectColorBarHistogram (true);
    
    connect (
	widgetGl,
	SIGNAL (ViewChanged ()),
	this,
	SLOT (ViewToUI ()));
}

void MainWindow::deformationViewToUI ()
{
    const ViewSettings& vs = widgetGl->GetViewSettings ();
    SetCheckedNoSignals (checkBoxShowDeformation, 
			 vs.IsDeformationShown ());
    SetCheckedNoSignals (checkBoxDeformationGrid, 
			 vs.GetDeformationAverage ().IsGridShown ());
    SetCheckedNoSignals (
	checkBoxDeformationGridCellCenter, 
	vs.GetDeformationAverage ().IsGridCellCenterShown ());
    SetValueNoSignals (
	horizontalSliderDeformationSize, 
	Value2ExponentIndex (horizontalSliderDeformationSize, 
		    GLWidget::SIZE_EXP2, vs.GetDeformationSize ()));
    SetValueNoSignals (
	horizontalSliderDeformationLineWidth, 
	Value2ExponentIndex (horizontalSliderDeformationLineWidth,
		     GLWidget::LINE_WIDTH_EXP2,
		     vs.GetDeformationLineWidth ()));
}

void MainWindow::velocityViewToUI ()
{
    const ViewSettings& vs = widgetGl->GetViewSettings ();
    SetCheckedNoSignals (checkBoxShowVelocity, vs.IsVelocityShown ());
    SetCheckedNoSignals (checkBoxVelocityGrid, 
			 vs.GetVelocityAverage ().IsGridShown ());
    SetValueNoSignals (
	horizontalSliderVelocitySize, 
	Value2ExponentIndex (horizontalSliderVelocitySize, 
		    GLWidget::SIZE_EXP2, vs.GetVelocitySize ()));
    SetValueNoSignals (
	horizontalSliderVelocityLineWidth, 
	Value2ExponentIndex (horizontalSliderVelocityLineWidth,
		    GLWidget::LINE_WIDTH_EXP2,
		    vs.GetVelocityLineWidth ()));
}

void MainWindow::forceViewToUI ()
{
    const ViewSettings& vs = widgetGl->GetViewSettings ();
    SetCheckedNoSignals (checkBoxForceNetwork, vs.IsForceNetworkShown ());
    SetCheckedNoSignals (checkBoxForcePressure, vs.IsForcePressureShown ());
    SetCheckedNoSignals (checkBoxForceResult, vs.IsForceResultShown ());    
    SetValueNoSignals (
	horizontalSliderForceSize, vs.GetForceSize ());
    SetValueNoSignals (
	horizontalSliderForceSize, 
	Value2ExponentIndex (horizontalSliderForceSize,
		   GLWidget::FORCE_SIZE_EXP2, vs.GetForceSize ()));
    SetValueNoSignals (
	horizontalSliderForceLineWidth, 
	Value2ExponentIndex (horizontalSliderForceLineWidth,
		   GLWidget::LINE_WIDTH_EXP2,
		   vs.GetForceLineWidth ()));
}

void MainWindow::t1sPDEViewToUI ()
{
    const ViewSettings& vs = widgetGl->GetViewSettings ();
    SetCheckedNoSignals (checkBoxTextureSizeShown, 
			 vs.GetT1sPDE ().IsKernelTextureSizeShown ());
    SetValueNoSignals (
	horizontalSliderT1sKernelTextureSize,
	Value2Index (horizontalSliderT1sKernelTextureSize,
		     T1sPDE::KERNEL_TEXTURE_SIZE,
		     vs.GetT1sPDE ().GetKernelTextureSize ()));
    SetValueNoSignals (
	horizontalSliderT1sKernelIntervalPerPixel,
	Value2Index(horizontalSliderT1sKernelIntervalPerPixel,
		    T1sPDE::KERNEL_INTERVAL_PER_PIXEL,
		    vs.GetT1sPDE ().GetKernelIntervalPerPixel ()));
    SetValueNoSignals (
	horizontalSliderT1sKernelSigma,
	Value2Index (horizontalSliderT1sKernelSigma,
		     T1sPDE::KERNEL_SIGMA,
		     vs.GetT1sPDE ().GetKernelSigma ()));
}

void MainWindow::ViewToUI ()
{
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    const ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    int property = vs.GetBodyOrFaceProperty ();
    size_t simulationIndex = vs.GetSimulationIndex ();
    ViewType::Enum viewType = vs.GetViewType ();

    SetCheckedNoSignals (buttonGroupViewType, viewType, true);    
    setStackedWidget (viewType);
    SetCheckedNoSignals (buttonGroupHistogram, 
			 (viewNumber == m_histogramViewNumber) ?
			 m_histogramType : HistogramType::NONE, true);

    SetCurrentIndexNoSignals (comboBoxColor, property);
    SetCurrentIndexNoSignals (comboBoxSimulation, simulationIndex);
    SetCurrentIndexNoSignals (comboBoxStatisticsType, vs.GetStatisticsType ());
    SetCurrentIndexNoSignals (comboBoxAxesOrder, vs.GetAxesOrder ());

    SetCheckedNoSignals (checkBoxContextHidden, vs.IsContextHidden ());
    SetCheckedNoSignals (checkBoxCenterPathHidden, vs.IsCenterPathHidden ());
    SetCheckedNoSignals (checkBoxT1sShiftLower, vs.IsT1sShiftLower ());

    deformationViewToUI ();
    velocityViewToUI ();
    forceViewToUI ();
    t1sPDEViewToUI ();

    SetValueNoSignals (horizontalSliderAngleOfView, vs.GetAngleOfView ());
    SetValueAndMaxNoSignals (spinBoxStatisticsTimeWindow,
			     vs.GetScalarAverage ().GetTimeWindow (), 
			     widgetGl->GetTimeSteps (viewNumber));    
    if (widgetGl->GetTimeLinkage () == TimeLinkage::INDEPENDENT)
    {
	sliderTimeSteps->SetValueAndMaxNoSignals (
	    vs.GetCurrentTime (), widgetGl->GetTimeSteps (viewNumber) - 1);
	labelStatisticsLinkedTimeWindowTitle->setHidden (true);
	labelStatisticsLinkedTimeWindow->setHidden (true);
    }
    else
    {
	size_t steps = widgetGl->LinkedTimeMaxSteps ().first;
	sliderTimeSteps->SetValueAndMaxNoSignals (widgetGl->GetLinkedTime (), 
						  steps - 1);
	labelStatisticsLinkedTimeWindowTitle->setHidden (false);
	labelStatisticsLinkedTimeWindow->setHidden (false);
	ostringstream ostr;
	ostr << vs.GetScalarAverage ().GetTimeWindow () * 
	    widgetGl->LinkedTimeStepStretch (viewNumber);
	labelStatisticsLinkedTimeWindow->setText (ostr.str ().c_str ());
    }

    labelFacesStatisticsColor->setText (BodyOrFacePropertyToString (property));
    labelCenterPathColor->setText (BodyOrFacePropertyToString (property));

    ostringstream ostr;
    ostr << vs.GetLinkedTimeBegin ();
    labelLinkedTimeBegin->setText (ostr.str ().c_str ());
    ostr.str ("");
    ostr << vs.GetLinkedTimeEnd ();
    labelLinkedTimeEnd->setText (ostr.str ().c_str ());
    ostr.str ("");
    ostr << widgetGl->GetTimeSteps (viewNumber);
    labelLinkedTimeSteps->setText (ostr.str ().c_str ());
    ostr.str ("");
    ostr << setprecision (3) << widgetGl->LinkedTimeStepStretch (viewNumber);
    labelLinkedTimeStepStretch->setText (ostr.str ().c_str ());

    updateLightControls (vs, selectedLight);
    updateButtons ();
}

void MainWindow::connectColorBarHistogram (bool connected)
{
    if (connected)
    {
	connect (
	    this, 
	    SIGNAL (BodyOrFacePropertyChanged (boost::shared_ptr<ColorBarModel>,
					       size_t)),
	    this, 
	    SLOT (SetHistogramColorBarModel (
		      boost::shared_ptr<ColorBarModel>)));
	connect (
	    this, 
	    SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	    this, 
	    SLOT (SetHistogramColorBarModel (
		      boost::shared_ptr<ColorBarModel>)), 
	    Qt::UniqueConnection);
	connect (
	    widgetGl, 
	    SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	    this, 
	    SLOT (SetHistogramColorBarModel (
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
    buttonGroupHistogram->setId (
	radioButtonHistogramNone, HistogramType::NONE);
    buttonGroupHistogram->setId (
	radioButtonHistogramUnicolor, HistogramType::UNICOLOR);
    buttonGroupHistogram->setId (
	radioButtonHistogramColorCoded, HistogramType::COLOR_CODED);
        
    buttonGroupViewType->setId (radioButtonEdgesNormal, ViewType::EDGES);
    buttonGroupViewType->setId (radioButtonEdgesTorus, ViewType::EDGES_TORUS);
    buttonGroupViewType->setId (radioButtonFaceEdgesTorus, 
				ViewType::FACES_TORUS);
    buttonGroupViewType->setId (radioButtonFacesNormal, ViewType::FACES);
    buttonGroupViewType->setId (radioButtonCenterPath, ViewType::CENTER_PATHS);
    buttonGroupViewType->setId (radioButtonFacesStatistics, 
				ViewType::FACES_STATISTICS);
    buttonGroupViewType->setId (radioButtonT1sPDE, ViewType::T1S_PDE);
    

    buttonGroupInteractionObject->setId (
	radioButtonInteractionFocus, InteractionObject::FOCUS);
    buttonGroupInteractionObject->setId (
	radioButtonInteractionLight, InteractionObject::LIGHT);
    buttonGroupInteractionObject->setId (
	radioButtonInteractionContext, InteractionObject::CONTEXT);
    buttonGroupInteractionObject->setId (
	radioButtonInteractionGrid, InteractionObject::GRID);

    buttonGroupTimeLinkage->setId (
	radioButtonTimeIndependent, TimeLinkage::INDEPENDENT);
    buttonGroupTimeLinkage->setId (
	radioButtonTimeLinked, TimeLinkage::LINKED);
    buttonGroupTimeLinkage->button (TimeLinkage::LINKED)->setChecked (true);

    buttonGroupTransformLinkage->setId (
	radioButtonTransformIndependent, TransformLinkage::INDEPENDENT);
    buttonGroupTransformLinkage->setId (
	radioButtonTransformLinkedHorizontalAxis, 
	TransformLinkage::LINKED_HORIZONTAL_AXIS);
}

void MainWindow::setupSliderData (const Simulation& simulation)
{
    sliderTimeSteps->setMinimum (0);
    sliderTimeSteps->setMaximum (simulation.GetTimeSteps () - 1);
    sliderTimeSteps->setSingleStep (1);
    sliderTimeSteps->setPageStep (10);
}

void MainWindow::setupHistogram ()
{
    // does not work
    QwtScaleWidget* yLeftAxis = widgetHistogram->axisWidget (QwtPlot::yLeft);
    yLeftAxis->setBorderDist (100, 100);
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
	Foam& currentFoam = const_cast<Foam&> (
	    widgetGl->GetSimulation ().GetFoam (0));
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
	widgetGl->update ();
    }
}

void MainWindow::processBodyTorusStep ()
{
    try
    {
	const Foam& currentFoam = widgetGl->GetSimulation ().GetFoam (0);
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
		    widgetGl->GetSimulation ().GetFoam (0).GetBodies ().size ();
	    }
	}
	widgetGl->update ();
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
    }
}


void MainWindow::SetAndDisplayHistogram (
    HistogramSelection histogramSelection,
    MaxValueOperation maxValueOperation)
{
    switch (m_histogramType)
    {
    case HistogramType::NONE:
	widgetHistogram->setHidden (true);
	return;
    case HistogramType::UNICOLOR:
	widgetHistogram->setVisible (true);
	widgetHistogram->SetColorCoded (false);
	break;
    case HistogramType::COLOR_CODED:
	widgetHistogram->setVisible (true);
	widgetHistogram->SetColorCoded (true);
	break;
    default:
	RuntimeAssert (false, "Invalid histogram type");
	return;
    }
    const ViewSettings& vs = widgetGl->GetViewSettings (m_histogramViewNumber);
    BodyProperty::Enum property = BodyProperty::FromSizeT (
	vs.GetBodyOrFaceProperty ());
    ViewType::Enum viewType = vs.GetViewType ();
    const Simulation& simulation = widgetGl->GetSimulation ();
    double maxYValue = 0;
    QwtIntervalData intervalData;
    if (viewType == ViewType::CENTER_PATHS)
    {
	const HistogramStatistics& allTimestepsHistogram = 
	    simulation.GetHistogram (property);
	intervalData = allTimestepsHistogram.ToQwtIntervalData ();
	maxYValue = allTimestepsHistogram.GetMaxCountPerBin ();
    }
    else
    {
	intervalData = simulation.GetFoam (widgetGl->GetCurrentTime ()).
	    GetHistogram (property).ToQwtIntervalData ();
	if (maxValueOperation == REPLACE_MAX_VALUE)
	    maxYValue = simulation.GetMaxCountPerBinIndividual (property);
    }
    if (maxValueOperation == KEEP_MAX_VALUE)
	maxYValue = widgetHistogram->GetMaxValueAxis ();
    if (histogramSelection == KEEP_SELECTION)
	widgetHistogram->SetDataKeepBinSelection (
	    intervalData, maxYValue, BodyProperty::ToString (property));
    else
	widgetHistogram->SetDataAllBinsSelected (
	    intervalData, maxYValue, BodyProperty::ToString (property));
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
    connect(widgetGl->GetActionEditColorMap ().get (), SIGNAL(triggered()),
	    this, SLOT(ShowEditColorMap ()));
    addAction (sliderTimeSteps->GetActionNextSelectedTimeStep ().get ());
    addAction (sliderTimeSteps->GetActionPreviousSelectedTimeStep ().get ());

    addAction (m_actionRotateShown.get ());
    addAction (m_actionTranslateShown.get ());
    addAction (m_actionScaleShown.get ());
    addAction (m_actionSelectShown.get ());
    addAction (m_actionDeselectShown.get ());
}


/**
 * @todo Calculate the correct histogram for the 'domain histogram' image.
 */
MainWindow::HistogramInfo MainWindow::getCurrentHistogramInfo () const
{
    const Simulation& simulation = widgetGl->GetSimulation ();
    switch (widgetGl->GetColorBarType ())
    {
    case ColorBarType::STATISTICS_COUNT:
	return createHistogramInfo (widgetGl->GetRangeCount (), 1);
    
    case ColorBarType::PROPERTY:
    {
	const HistogramStatistics& histogramStatistics = 
	    simulation.GetHistogram (widgetGl->GetBodyOrFaceProperty ());
	return HistogramInfo (histogramStatistics.ToQwtIntervalData (), 
			      histogramStatistics.GetMaxCountPerBin ());
    }
    
    case ColorBarType::T1S_PDE:
	return createHistogramInfo (
	    widgetGl->GetRangeT1sPDE (), simulation.GetT1sSize ());

    default:
	ThrowException ("Invalid call to getCurrentHistogramInfo");
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


void MainWindow::displayHistogramColorBar (bool checked)
{
    widgetHistogram->setVisible (
	checked && 
	widgetGl->GetBodyOrFaceProperty () != FaceProperty::DMP_COLOR && 
	m_histogramType);
}

void MainWindow::setupColorBarModels ()
{
    size_t simulationCount = widgetGl->GetSimulationGroup ().size ();
    m_colorBarModelBodyProperty.resize (simulationCount);
    m_colorBarModelDomainHistogram.resize (simulationCount);
    m_colorBarModelT1sPDE.resize (simulationCount);
    for (size_t simulationIndex = 0; simulationIndex < simulationCount; 
	 ++simulationIndex)
	for (size_t vn = 0; vn < ViewNumber::COUNT; ++vn)
	    setupColorBarModels (simulationIndex, ViewNumber::Enum (vn));    
}

void MainWindow::setupViews ()
{
    for (size_t i = 0; i < widgetGl->GetViewSettingsSize (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	widgetGl->SetBodyOrFaceProperty (
	    viewNumber, 
	    m_colorBarModelBodyProperty[0][viewNumber][BodyProperty::PRESSURE], 
	    BodyProperty::PRESSURE);
    }
}

void MainWindow::setupColorBarModels (size_t simulationIndex,
				      ViewNumber::Enum viewNumber)
{
    size_t i = 0;
    BOOST_FOREACH (boost::shared_ptr<ColorBarModel>& colorBarModel,
		   m_colorBarModelBodyProperty[simulationIndex][viewNumber])
    {
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	colorBarModel.reset (new ColorBarModel ());
	setupColorBarModel (simulationIndex, viewNumber, property);
	++i;
    }

    {
	boost::shared_ptr<ColorBarModel>& colorBarModel = 
	    m_colorBarModelDomainHistogram[simulationIndex][viewNumber];
	colorBarModel.reset (new ColorBarModel ());
	colorBarModel->SetTitle ("Count per area");
	colorBarModel->SetInterval (
	    toQwtDoubleInterval (widgetGl->GetRangeCount ()));
	colorBarModel->SetupPalette (Palette::BLACK_BODY);
    }

    {
	boost::shared_ptr<ColorBarModel>& colorBarModel = 
	    m_colorBarModelT1sPDE[simulationIndex][viewNumber];
	colorBarModel.reset (new ColorBarModel ());
	colorBarModel->SetTitle ("T1s PDE");
	colorBarModel->SetInterval (
	    toQwtDoubleInterval (widgetGl->GetRangeT1sPDE (viewNumber)));
	colorBarModel->SetupPalette (Palette::BLACK_BODY);
    }
}

void MainWindow::setupColorBarModel (size_t simulationIndex,
				     ViewNumber::Enum viewNumber,
				     BodyProperty::Enum property)
{
    const Simulation& simulation = 
	widgetGl->GetSimulation (simulationIndex);
    m_colorBarModelBodyProperty[simulationIndex][viewNumber][property]
	->SetTitle (BodyProperty::ToString (property));
    m_colorBarModelBodyProperty[simulationIndex][viewNumber][property]
	->SetInterval (simulation.GetRange (property));
    m_colorBarModelBodyProperty[simulationIndex][viewNumber][property]
	->SetupPalette (Palette::BLUE_RED_DIVERGING);
}

void MainWindow::updateLightControls (
    const ViewSettings& vs, LightNumber::Enum lightNumber)
{
    checkBoxLightEnabled->setChecked (vs.IsLightEnabled (lightNumber));
    checkBoxLightPositionShown->setChecked (
	vs.IsLightPositionShown (lightNumber));
    checkBoxDirectionalLightEnabled->setChecked (
	vs.IsDirectionalLightEnabled (lightNumber));
    horizontalSliderLightAmbientRed->setValue (
	floor (vs.GetLight (lightNumber, LightType::AMBIENT)[0] * 
	       horizontalSliderLightAmbientRed->maximum () + 0.5));
    horizontalSliderLightAmbientGreen->setValue (
	floor (vs.GetLight (lightNumber, LightType::AMBIENT)[1] * 
	       horizontalSliderLightAmbientGreen->maximum () + 0.5));
    horizontalSliderLightAmbientBlue->setValue (
	floor (vs.GetLight (lightNumber, LightType::AMBIENT)[2] * 
	       horizontalSliderLightAmbientBlue->maximum () + 0.5));
    horizontalSliderLightDiffuseRed->setValue (
	floor (vs.GetLight (lightNumber, LightType::DIFFUSE)[0] * 
	       horizontalSliderLightDiffuseRed->maximum () + 0.5));
    horizontalSliderLightDiffuseGreen->setValue (
	floor (vs.GetLight (lightNumber, LightType::DIFFUSE)[1] * 
	       horizontalSliderLightDiffuseGreen->maximum () + 0.5));
    horizontalSliderLightDiffuseBlue->setValue (
	floor (vs.GetLight (lightNumber, LightType::DIFFUSE)[2] * 
	       horizontalSliderLightDiffuseBlue->maximum () + 0.5));
    horizontalSliderLightSpecularRed->setValue (
	floor (vs.GetLight (
		   lightNumber, LightType::SPECULAR)[0] * 
	       horizontalSliderLightSpecularRed->maximum () + 0.5));
    horizontalSliderLightSpecularGreen->setValue (
	floor (vs.GetLight (
		   lightNumber, LightType::SPECULAR)[1] * 
	       horizontalSliderLightSpecularGreen->maximum () + 0.5));
    horizontalSliderLightSpecularBlue->setValue (
	floor (vs.GetLight (
		   lightNumber, LightType::SPECULAR)[2] * 
	    horizontalSliderLightSpecularBlue->maximum () + 0.5));
}

boost::shared_ptr<ColorBarModel> MainWindow::getColorBarModel (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber,
    ViewType::Enum viewType, size_t property, 
    StatisticsType::Enum statisticsType) const
{
    ColorBarType::Enum colorBarType = GLWidget::GetColorBarType (
	viewType, property, statisticsType);
    switch (colorBarType)
    {
    case ColorBarType::T1S_PDE:
	return m_colorBarModelT1sPDE[simulationIndex][viewNumber];
    case ColorBarType::STATISTICS_COUNT:
	return m_colorBarModelDomainHistogram[simulationIndex][viewNumber];
    case ColorBarType::PROPERTY:
	return m_colorBarModelBodyProperty
	    [simulationIndex][viewNumber][property];
    default:
	return boost::shared_ptr<ColorBarModel> ();
    }
}

boost::shared_ptr<ColorBarModel> MainWindow::getColorBarModel () const
{
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    size_t simulationIndex = 
	widgetGl->GetViewSettings (viewNumber).GetSimulationIndex ();
    ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceProperty ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    return getColorBarModel (simulationIndex, 
			     viewNumber, viewType, property, statisticsType);
}


void MainWindow::emitColorBarModelChanged (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber,
    ViewType::Enum viewType, 
    size_t property, StatisticsType::Enum statisticsType)
{
    Q_EMIT ColorBarModelChanged (
	getColorBarModel (simulationIndex, 
			  viewNumber, viewType, property, statisticsType));
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


// Slots
// ======================================================================

void MainWindow::ToggledHistogramGridShown (bool checked)
{
    widgetHistogram->SetGridEnabled (checked);
}

void MainWindow::ClickedPlay ()
{
    clickedPlay (PLAY_FORWARD);
}

void MainWindow::ClickedPlayReverse ()
{
    clickedPlay (PLAY_REVERSE);
}

void MainWindow::ClickedBegin ()
{
    sliderTimeSteps->setValue (sliderTimeSteps->minimum ());
    updateButtons ();
}

void MainWindow::ClickedEnd ()
{
    sliderTimeSteps->SetValueNoSignals (sliderTimeSteps->maximum ());
    updateButtons ();
}

void MainWindow::TimeoutTimer ()
{
    //cdbg << "TimeoutTimer" << endl;
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

void MainWindow::ValueChangedFontSize (int fontSize)
{
    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    defaultFont.setPointSize (fontSize);
    app->setFont (defaultFont);
    widgetHistogram->SetDefaultFont ();
    m_editColorMap->SetDefaultFont ();
}

void MainWindow::ValueChangedHistogramHeight (int s)
{
    widgetHistogram->SetSizeHint (QSize(s, s));
    widgetHistogram->updateGeometry ();
}

void MainWindow::ValueChangedT1sKernelSigma (int index)
{
    (void)index;
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    size_t simulationIndex = 
	widgetGl->GetViewSettings (viewNumber).GetSimulationIndex ();
    m_colorBarModelT1sPDE[simulationIndex][viewNumber]->SetInterval (
	toQwtDoubleInterval (widgetGl->GetRangeT1sPDE (viewNumber)));
}

void MainWindow::ValueChangedSliderTimeSteps (int timeStep)
{
    (void)timeStep;
    SetAndDisplayHistogram (KEEP_SELECTION, KEEP_MAX_VALUE);
    Foam& foam = const_cast<Foam&>
	(widgetGl->GetSimulation ().GetFoam (0));
    if (m_debugTranslatedBody)
	m_currentTranslatedBody = foam.GetBodies ().begin ();
    updateButtons ();
}

void MainWindow::ValueChangedStatisticsTimeWindow (int timeSteps)
{
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    ostringstream ostr;
    ostr << timeSteps * widgetGl->LinkedTimeStepStretch (viewNumber);
    labelStatisticsLinkedTimeWindow->setText (ostr.str ().c_str ());
}


void MainWindow::ButtonClickedViewType (int vt)
{
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    const Simulation& simulation = widgetGl->GetSimulation (viewNumber);
    ViewType::Enum viewType = ViewType::Enum(vt);
    size_t simulationIndex = 
	widgetGl->GetViewSettings (viewNumber).GetSimulationIndex ();
    ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
    ViewType::Enum oldViewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceProperty ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();

    setStackedWidget (viewType);
    emitColorBarModelChanged (
	simulationIndex, viewNumber, viewType, property, statisticsType);
    switch (viewType)
    {
    case ViewType::FACES:
	if (m_histogramViewNumber == viewNumber)
	    ButtonClickedHistogram (m_histogramType);
	break;

    case ViewType::FACES_STATISTICS:
	labelFacesStatisticsColor->setText (
	    BodyProperty::ToString (BodyProperty::FromSizeT (property)));
	if (m_histogramViewNumber == viewNumber)
	    ButtonClickedHistogram (m_histogramType);
	break;

    case ViewType::CENTER_PATHS:
	labelCenterPathColor->setText (
	    BodyProperty::ToString (BodyProperty::FromSizeT (property)));
	if (m_histogramViewNumber == viewNumber)
	    ButtonClickedHistogram (m_histogramType);
	break;

    case ViewType::T1S_PDE:
	sliderTimeSteps->setMaximum (simulation.GetT1sTimeSteps () - 1);
	break;

    default:
	break;
    }
    if (oldViewType == ViewType::T1S_PDE)
	sliderTimeSteps->setMaximum (simulation.GetTimeSteps () - 1);
}






void MainWindow::setStackedWidget (ViewType::Enum viewType)
{
    // WARNING: Has to match ViewType::Enum order
    QWidget* pages[] = 
	{
	    pageEdgesNormal,
	    pageTimeStepEmpty,
	    pageTimeStepEmpty,
	    pageFacesNormal,

	    pageCenterPath,
	    pageFacesStatistics,
	    pageT1sProbabilityDensity
	};
    if (ViewType::IsTimeDependent (viewType))
    {
	stackedWidgetTimeStep->setCurrentWidget (pageTimeStepEmpty);
	stackedWidgetTimeDependent->setCurrentWidget (pages[viewType]);
    }
    else
    {
	stackedWidgetTimeStep->setCurrentWidget (pages[viewType]);
	stackedWidgetTimeDependent->setCurrentWidget (pageTimeDependentEmpty);
    }
}

void MainWindow::CurrentIndexChangedSimulation (int index)
{
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    ViewSettings& vs = widgetGl->GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceProperty ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    emitColorBarModelChanged (
	index, viewNumber, viewType, property, statisticsType);
    ViewToUI ();
}

void MainWindow::CurrentIndexChangedSelectedLight (int i)
{
    const ViewSettings& vs = widgetGl->GetViewSettings ();
    LightNumber::Enum lightNumber = LightNumber::Enum (i);
    updateLightControls (vs, lightNumber);
}

void MainWindow::CurrentIndexChangedFaceColor (int value)
{
    boost::array<QWidget*, 4> widgetsVisible = {{
	    labelFacesHistogram, radioButtonHistogramNone,
	    radioButtonHistogramUnicolor, radioButtonHistogramColorCoded}};
    boost::array<QWidget*, 1> widgetsEnabled = {{radioButtonFacesStatistics}};
    ViewNumber::Enum viewNumber = widgetGl->GetViewNumber ();
    size_t simulationIndex = 
	widgetGl->GetViewSettings (viewNumber).GetSimulationIndex ();
    if (value == FaceProperty::DMP_COLOR) {
	::setVisible (widgetsVisible, false);
	::setEnabled (widgetsEnabled, false);
	Q_EMIT BodyOrFacePropertyChanged (
	    boost::shared_ptr<ColorBarModel> (), value);
    }
    else {
	BodyProperty::Enum property = BodyProperty::FromSizeT (value);
	::setVisible (widgetsVisible, true);
	::setEnabled (widgetsEnabled, true);
	Q_EMIT BodyOrFacePropertyChanged (
	    m_colorBarModelBodyProperty
	    [simulationIndex][viewNumber][property], property);
	SetAndDisplayHistogram (DISCARD_SELECTION, REPLACE_MAX_VALUE);
    }
}

void MainWindow::CurrentIndexChangedViewCount (int index)
{
    boost::array<QWidget*, 2> widgetsViewLayout = 
	{{labelViewLayout, comboBoxViewLayout}};
    ViewCount::Enum viewCount = ViewCount::Enum (index);
    if (viewCount == ViewCount::TWO || viewCount == ViewCount::THREE)
	::setVisible (widgetsViewLayout, true);
    else
	::setVisible (widgetsViewLayout, false);
    checkBoxTitleShown->setChecked (viewCount != ViewCount::ONE);
}


void MainWindow::CurrentIndexChangedStatisticsType (int value)
{
    boost::array<QWidget*, 2> widgetsStatisticsTimeWindow = 
	{{ spinBoxStatisticsTimeWindow, labelStatisticsTimeWindow}};
    switch (value)
    {
    case StatisticsType::AVERAGE:
	connectColorBarHistogram (true);
	::setVisible (widgetsStatisticsTimeWindow, true);
	break;
    case StatisticsType::MIN:
    case StatisticsType::MAX:
	connectColorBarHistogram (true);
	::setVisible (widgetsStatisticsTimeWindow, false);
	break;
    case StatisticsType::COUNT:
	connectColorBarHistogram (false);
	::setVisible (widgetsStatisticsTimeWindow, false);
	break;
    }
    Q_EMIT ColorBarModelChanged (getColorBarModel ());
}

void MainWindow::CurrentIndexChangedWindowSize (int value)
{
    WindowSize::Enum windowSize = WindowSize::Enum (value);
    switch (windowSize)
    {
    case WindowSize::WINDOW_720x480:
	resize (720, 480);
	break;
    case WindowSize::GL_720x480:
	resize (1109, 612);
	break;
    }
}


void MainWindow::ButtonClickedHistogram (int histogramType)
{
    m_histogramType = HistogramType::Enum (histogramType);
    m_histogramViewNumber = widgetGl->GetViewNumber ();
    SetAndDisplayHistogram (KEEP_SELECTION, REPLACE_MAX_VALUE);
}

void MainWindow::ButtonClickedTransformLinkage (int id)
{
    TransformLinkage::Enum transformLinkage = TransformLinkage::Enum (id);
    if (transformLinkage == TransformLinkage::LINKED_HORIZONTAL_AXIS &&
	(widgetGl->GetViewCount () != ViewCount::TWO || 
	 widgetGl->GetViewLayout () != ViewLayout::VERTICAL))
    {
	QMessageBox msgBox (this);
	msgBox.setText("This feature works only with two views "
		       "in vertical layout");
	msgBox.exec();
	SetCheckedNoSignals (radioButtonTransformIndependent, true);
	return;
    }
    widgetGl->SetTransformLinkage (transformLinkage);
}


void MainWindow::SelectionChangedHistogram ()
{
    vector<QwtDoubleInterval> valueIntervals;
    widgetHistogram->GetSelectedIntervals (&valueIntervals);
    vector<bool> timeStepSelection;
    const Simulation& simulation = widgetGl->GetSimulation ();
    simulation.GetTimeStepSelection (
	BodyProperty::FromSizeT (
	    widgetGl->GetBodyOrFaceProperty (m_histogramViewNumber)), 
	valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);
    
    if (widgetHistogram->AreAllItemsSelected ())
	widgetGl->GetViewSettings (m_histogramViewNumber).SetBodySelector (
	    AllBodySelector::Get (), BodySelectorType::PROPERTY_VALUE);
    else
	widgetGl->GetViewSettings (m_histogramViewNumber).SetBodySelector (
	    boost::shared_ptr<PropertyValueBodySelector> (
		new PropertyValueBodySelector (
		    BodyProperty::FromSizeT (
			widgetGl->GetBodyOrFaceProperty (
			    m_histogramViewNumber)), 
		    valueIntervals)));
    widgetGl->update ();
}

void MainWindow::ShowEditColorMap ()
{
    HistogramInfo p = getCurrentHistogramInfo ();
    m_editColorMap->SetData (
	p.first, p.second, *getColorBarModel (),
	checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*getColorBarModel () = m_editColorMap->GetColorBarModel ();
	Q_EMIT ColorBarModelChanged (getColorBarModel ());
    }
}

void MainWindow::SetHistogramColorBarModel (
    boost::shared_ptr<ColorBarModel> colorBarModel)
{
    if (m_histogramViewNumber == widgetGl->GetViewNumber ())
	widgetHistogram->SetColorBarModel (colorBarModel);
}

void MainWindow::CurrentIndexChangedInteractionMode (int index)
{
    InteractionMode::Enum im = InteractionMode::Enum(index);
    radioButtonInteractionFocus->click ();
    radioButtonInteractionLight->setDisabled (true);
    radioButtonInteractionContext->setDisabled (true);
    radioButtonInteractionGrid->setDisabled (true);
    switch (im)
    {
    case InteractionMode::ROTATE:
	radioButtonInteractionLight->setEnabled (true);
	break;
	
    case InteractionMode::SCALE:
	radioButtonInteractionContext->setEnabled (true);
	radioButtonInteractionGrid->setEnabled (true);
	break;
	
    case InteractionMode::TRANSLATE:
	radioButtonInteractionLight->setEnabled (true);
	radioButtonInteractionGrid->setEnabled (true);
	break;
    
    default:
	break;
    }
}
