/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
// @todo Store the palette per BodyProperty instead of storing it per view 
//       and simulation index.
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
    glWidget->SetStatus (labelStatusBar);
    glWidget->SetSimulationGroup (&simulationGroup);
    setupColorBarModels ();
    setupViews ();
    initComboBoxSimulation (simulationGroup);
    configureInterface ();
    setupHistogram ();
    createActions ();
    setTabOrder (radioButtonBubblesPaths, sliderTimeSteps);
    string title ("FoamVis");
    if (simulationGroup.size () == 1)
    {
	Simulation& simulation = simulationGroup.GetSimulation (0);
	title += " - ";
	title += simulation.GetName ();
    }
    setWindowTitle (QString (title.c_str ()));
    widgetHistogram->setHidden (true);
    m_timer->setInterval (20);
    //initTranslatedBody ();
    configureInterfaceDataDependent (simulationGroup);    
}

void MainWindow::configureInterface ()
{
    tabWidget->setCurrentWidget (tabTimeStep);
    comboBoxWindowSize->setCurrentIndex (WindowSize::GL_720x480);
    horizontalSliderForceTorqueSize->setValue (49);
    horizontalSliderTorqueDistance->setValue (49);
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
	radioButtonT1sPDE->setEnabled (true);
    }
    if (! simulation.IsTorus ())
    {
	checkBoxTorusOriginalDomain->setDisabled (true);
	checkBoxTorusOriginalDomainWrapInside->setDisabled (true);
	radioButtonEdgesTorus->setDisabled (true);
	radioButtonFaceEdgesTorus->setDisabled (true);
    }
    if (simulation.Is2D ())
    {
	comboBoxInteractionMode->setCurrentIndex (InteractionMode::SCALE);
	if (simulation.IsTorus ())
	{
	    G3D::AABox box = simulation.GetBoundingBox ();
	    G3D::Vector3 extent = box.extent ();
	    if (extent.x > extent.y && simulation.GetRotation2D () == 0)
		comboBoxViewLayout->setCurrentIndex (ViewLayout::VERTICAL);
	}
	if (! simulation.ForcesUsed ())
	{
	    checkBoxForceNetwork->setDisabled (true);
	    checkBoxForcePressure->setDisabled (true);
	    checkBoxForceResult->setDisabled (true);
	}
    }
    else
    {
	comboBoxAxesOrder->setCurrentIndex (AxesOrder::THREE_D);
	checkBoxLightEnabled->setChecked (true);
    }
    size_t viewCount = min (simulationGroup.size (),
			    ViewCount::GetCount (ViewCount::MAX));
    if (simulationGroup.size () > 1)
	comboBoxViewCount->setCurrentIndex (ViewCount::FromSizeT (viewCount));
    for (size_t i = 1; i < viewCount; ++i)
    {
	glWidget->SetViewNumber (ViewNumber::Enum (i));
	comboBoxSimulation->setCurrentIndex (i);
    }
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
	(glWidget->GetSimulation ().GetFoam (0));
    m_currentTranslatedBody = foam.GetBodies ().begin ();
}


void MainWindow::connectSignals ()
{
    
    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));
    
    connect (glWidget, SIGNAL (PaintedGL ()),
	     widgetDisplay, SLOT (SaveFrame ()), 
	     Qt::QueuedConnection);
    
    
    // BodyOrFacePropertyChanged: 
    // from MainWindow to GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (BodyOrFacePropertyChanged (
		    ViewNumber::Enum,
		    boost::shared_ptr<ColorBarModel>, size_t)),
	glWidget, 
	SLOT (SetBodyOrFaceProperty (
		  ViewNumber::Enum, boost::shared_ptr<ColorBarModel>, size_t)));
    
    // ColorBarModelChanged:
    // from MainWindow to ColorBar, GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (ViewNumber::Enum, 
				      boost::shared_ptr<ColorBarModel>)),
	glWidget, 
	SLOT (SetColorBarModel (ViewNumber::Enum, 
				boost::shared_ptr<ColorBarModel>)));
    connect (
	this, 
	SIGNAL (OverlayBarModelChanged (ViewNumber::Enum, 
					boost::shared_ptr<ColorBarModel>)),
	glWidget, 
	SLOT (SetOverlayBarModel (ViewNumber::Enum, 
					  boost::shared_ptr<ColorBarModel>)));

    
    
    // ColorBarModelChanged:
    // from GLWidget to GLWidget and MainWindow and AttributeHistogram
    connect (
	glWidget, 
	SIGNAL (ColorBarModelChanged (ViewNumber::Enum,
				      boost::shared_ptr<ColorBarModel>)),
	glWidget, 
	SLOT (SetColorBarModel (ViewNumber::Enum,
				boost::shared_ptr<ColorBarModel>)));
    connect (
	glWidget, 
	SIGNAL (OverlayBarModelChanged (ViewNumber::Enum,
					boost::shared_ptr<ColorBarModel>)),
	glWidget, 
	SLOT (SetOverlayBarModel (ViewNumber::Enum,
				  boost::shared_ptr<ColorBarModel>)));
    connectColorBarHistogram (true);
    
    connect (
	glWidget,
	SIGNAL (ViewChanged ()),
	this,
	SLOT (ViewToUI ()));
}

void MainWindow::deformationViewToUI ()
{
    const ViewSettings& vs = glWidget->GetViewSettings ();
    SetCheckedNoSignals (checkBoxDeformationShown, 
			 vs.IsDeformationShown ());
    SetCheckedNoSignals (checkBoxDeformationGridShown, 
			 vs.GetDeformationAverage ().IsGridShown ());
    SetCheckedNoSignals (
	checkBoxDeformationGridCellCenterShown, 
	vs.GetDeformationAverage ().IsGridCellCenterShown ());
    SetValueNoSignals (
	horizontalSliderDeformationSize, 
	Value2ExponentIndex (horizontalSliderDeformationSize, 
		    GLWidget::TENSOR_SIZE_EXP2, vs.GetDeformationSize ()));
    SetValueNoSignals (
	horizontalSliderDeformationLineWidth, 
	Value2ExponentIndex (horizontalSliderDeformationLineWidth,
		     GLWidget::TENSOR_LINE_WIDTH_EXP2,
		     vs.GetDeformationLineWidth ()));
}

void MainWindow::velocityViewToUI ()
{
    const ViewSettings& vs = glWidget->GetViewSettings ();
    const VectorAverage& va = vs.GetVelocityAverage ();
    SetCheckedNoSignals (checkBoxVelocityShown, vs.IsVelocityShown ());
    SetCheckedNoSignals (checkBoxVelocityGridShown, va.IsGridShown ());
    SetCheckedNoSignals (checkBoxVelocityClampingShown, va.IsClampingShown ());
    SetCheckedNoSignals (checkBoxVelocityGridCellCenterShown, 
			 va.IsGridCellCenterShown ());
    SetCheckedNoSignals (checkBoxVelocitySameSize, va.IsSameSize ());
    SetCheckedNoSignals (checkBoxVelocityColorMapped, va.IsColorMapped ());
    SetValueNoSignals (
	horizontalSliderVelocityLineWidth, 
	Value2ExponentIndex (horizontalSliderVelocityLineWidth,
		    GLWidget::TENSOR_LINE_WIDTH_EXP2,
		    vs.GetVelocityLineWidth ()));
}

void MainWindow::forceViewToUI ()
{
    ViewNumber::Enum viewNumber = glWidget->GetViewNumber ();
    const ViewSettings& vs = glWidget->GetViewSettings (viewNumber);
    SetCheckedNoSignals (
	checkBoxForceNetwork, vs.IsForceNetworkShown (),
	glWidget->GetSimulation (viewNumber).ForcesUsed ());
    SetCheckedNoSignals (
	checkBoxForcePressure, 
	vs.IsForcePressureShown (), 
	glWidget->GetSimulation (viewNumber).ForcesUsed ());
    SetCheckedNoSignals (
	checkBoxForceResult, vs.IsForceResultShown (),
	glWidget->GetSimulation (viewNumber).ForcesUsed ());
    SetValueNoSignals (
	horizontalSliderForceTorqueSize, 
	Value2ExponentIndex (horizontalSliderForceTorqueSize,
		   GLWidget::FORCE_SIZE_EXP2, vs.GetForceTorqueSize ()));
    SetValueNoSignals (
	horizontalSliderForceTorqueLineWidth, 
	Value2ExponentIndex (horizontalSliderForceTorqueLineWidth,
		   GLWidget::TENSOR_LINE_WIDTH_EXP2,
		   vs.GetForceTorqueLineWidth ()));
}

void MainWindow::t1sPDEViewToUI ()
{
    const ViewSettings& vs = glWidget->GetViewSettings ();
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
    ViewNumber::Enum viewNumber = glWidget->GetViewNumber ();
    const ViewSettings& vs = glWidget->GetViewSettings (viewNumber);
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

    SetCheckedNoSignals (checkBoxSelectionContextShown, 
			 vs.IsSelectionContextShown ());
    SetCheckedNoSignals (checkBoxCenterPathHidden, vs.IsCenterPathHidden ());
    SetCheckedNoSignals (checkBoxT1sShiftLower, vs.IsT1sShiftLower ());

    deformationViewToUI ();
    velocityViewToUI ();
    forceViewToUI ();
    t1sPDEViewToUI ();

    SetValueNoSignals (horizontalSliderAngleOfView, vs.GetAngleOfView ());
    SetValueAndMaxNoSignals (spinBoxStatisticsTimeWindow,
			     vs.GetScalarAverage ().GetTimeWindow (), 
			     glWidget->GetTimeSteps (viewNumber));
    SetValueAndMaxNoSignals (spinBoxT1sTimeWindow,
			     vs.GetT1sPDE ().GetTimeWindow (), 
			     glWidget->GetTimeSteps (viewNumber));
    if (glWidget->GetTimeLinkage () == TimeLinkage::INDEPENDENT)
    {
	sliderTimeSteps->SetValueAndMaxNoSignals (
	    vs.GetCurrentTime (), glWidget->GetTimeSteps (viewNumber) - 1);
	labelStatisticsLinkedTimeWindowTitle->setHidden (true);
	labelStatisticsLinkedTimeWindow->setHidden (true);
    }
    else
    {
	size_t steps = glWidget->LinkedTimeMaxSteps ().first;
	sliderTimeSteps->SetValueAndMaxNoSignals (glWidget->GetLinkedTime (), 
						  steps - 1);
	labelStatisticsLinkedTimeWindowTitle->setHidden (false);
	labelStatisticsLinkedTimeWindow->setHidden (false);
	ostringstream ostr;
	ostr << vs.GetScalarAverage ().GetTimeWindow () * 
	    glWidget->LinkedTimeStepStretch (viewNumber);
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
    ostr << glWidget->GetTimeSteps (viewNumber);
    labelLinkedTimeSteps->setText (ostr.str ().c_str ());
    ostr.str ("");
    ostr << setprecision (3) << glWidget->LinkedTimeStepStretch (viewNumber);
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
	    SIGNAL (BodyOrFacePropertyChanged (
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
	connect (
	    glWidget, 
	    SIGNAL (ColorBarModelChanged (
			ViewNumber::Enum, boost::shared_ptr<ColorBarModel>)),
	    this, 
	    SLOT (SetHistogramColorBarModel (
		      ViewNumber::Enum, boost::shared_ptr<ColorBarModel>)), 
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
    buttonGroupViewType->setId (radioButtonBubblesPaths, ViewType::CENTER_PATHS);
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
	    glWidget->GetSimulation ().GetFoam (0));
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
	glWidget->update ();
    }
}

void MainWindow::processBodyTorusStep ()
{
    try
    {
	const Foam& currentFoam = glWidget->GetSimulation ().GetFoam (0);
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
		    glWidget->GetSimulation ().GetFoam (0).GetBodies ().size ();
	    }
	}
	glWidget->update ();
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
    const ViewSettings& vs = glWidget->GetViewSettings (m_histogramViewNumber);
    BodyProperty::Enum property = BodyProperty::FromSizeT (
	vs.GetBodyOrFaceProperty ());
    ViewType::Enum viewType = vs.GetViewType ();
    const Simulation& simulation = glWidget->GetSimulation ();
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
	intervalData = simulation.GetFoam (glWidget->GetCurrentTime ()).
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
    
    addAction (glWidget->GetActionResetTransformAll ().get ());    
    connect(glWidget->GetActionEditColorMap ().get (), SIGNAL(triggered()),
	    this, SLOT(ShowEditColorMap ()));
    connect(glWidget->GetActionEditOverlayMap ().get (), SIGNAL(triggered()),
	    this, SLOT(ShowEditOverlayMap ()));
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
MainWindow::HistogramInfo MainWindow::getHistogramInfo (
    ColorBarType::Enum colorBarType, size_t bodyOrFaceProperty) const
{
    const Simulation& simulation = glWidget->GetSimulation ();
    switch (colorBarType)
    {
    case ColorBarType::STATISTICS_COUNT:
	return createHistogramInfo (glWidget->GetRangeCount (), 1);
    
    case ColorBarType::PROPERTY:
    {
	const HistogramStatistics& histogramStatistics = 
	    simulation.GetHistogram (bodyOrFaceProperty);
	return HistogramInfo (histogramStatistics.ToQwtIntervalData (), 
			      histogramStatistics.GetMaxCountPerBin ());
    }
    
    case ColorBarType::T1S_PDE:
	return createHistogramInfo (
	    glWidget->GetRangeT1sPDE (), simulation.GetT1sSize ());

    default:
	ThrowException ("Invalid call to getHistogramInfo");
	return createHistogramInfo (glWidget->GetRangeCount (), 1);
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
	glWidget->GetBodyOrFaceProperty () != FaceProperty::DMP_COLOR && 
	m_histogramType);
}

void MainWindow::setupColorBarModels ()
{
    size_t simulationCount = glWidget->GetSimulationGroup ().size ();
    m_colorBarModelBodyProperty.resize (simulationCount);
    m_overlayBarModelVelocityVector.resize (simulationCount);
    m_colorBarModelDomainHistogram.resize (simulationCount);
    m_colorBarModelT1sPDE.resize (simulationCount);
    for (size_t simulationIndex = 0; simulationIndex < simulationCount; 
	 ++simulationIndex)
	for (size_t vn = 0; vn < ViewNumber::COUNT; ++vn)
	    setupColorBarModels (simulationIndex, ViewNumber::Enum (vn));    
}

void MainWindow::setupViews ()
{
    for (size_t i = 0; i < glWidget->GetViewSettingsSize (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	glWidget->SetBodyOrFaceProperty (
	    viewNumber, 
	    m_colorBarModelBodyProperty[0][viewNumber][BodyProperty::PRESSURE], 
	    BodyProperty::PRESSURE);
    }
}

void MainWindow::setupColorBarModels (size_t simulationIndex,
				      ViewNumber::Enum viewNumber)
{    
    for (size_t i = 0; i < BodyProperty::COUNT; ++i)
    {
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	setupColorBarModelBodyProperty (simulationIndex, viewNumber, property);
    }
    setupColorBarModelVelocityVector (simulationIndex, viewNumber);
    setupColorBarModelDomainHistogram (simulationIndex, viewNumber);
    setupColorBarModelT1sPDE (simulationIndex, viewNumber);
}

void MainWindow::setupColorBarModelT1sPDE (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_colorBarModelT1sPDE[simulationIndex][viewNumber];
    colorBarModel.reset (new ColorBarModel ());
    colorBarModel->SetTitle ("T1s PDE");
    colorBarModel->SetInterval (
	toQwtDoubleInterval (glWidget->GetRangeT1sPDE (viewNumber)));
    colorBarModel->SetupPalette (
	Palette (PaletteType::SEQUENTIAL, PaletteSequential::BLACK_BODY));
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
	toQwtDoubleInterval (glWidget->GetRangeCount ()));
    colorBarModel->SetupPalette (
	Palette (PaletteType::SEQUENTIAL, PaletteSequential::BLACK_BODY));
}


void MainWindow::setupColorBarModelBodyProperty (size_t simulationIndex,
						 ViewNumber::Enum viewNumber,
						 BodyProperty::Enum property)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_colorBarModelBodyProperty[simulationIndex][viewNumber][property];
    setupColorBarModel (colorBarModel, property, simulationIndex);
}

void MainWindow::setupColorBarModelVelocityVector (
    size_t simulationIndex,
    ViewNumber::Enum viewNumber)
{
    boost::shared_ptr<ColorBarModel>& colorBarModel = 
	m_overlayBarModelVelocityVector[simulationIndex][viewNumber];
    BodyProperty::Enum property = BodyProperty::VELOCITY_MAGNITUDE;
    setupColorBarModel (colorBarModel, property, simulationIndex);
    colorBarModel->SetTitle ("Velocity overlay");
}

void MainWindow::setupColorBarModel (
    boost::shared_ptr<ColorBarModel>& colorBarModel, 
    BodyProperty::Enum property, size_t simulationIndex)
{
    colorBarModel.reset (new ColorBarModel ());
    const Simulation& simulation = glWidget->GetSimulation (simulationIndex);
    colorBarModel->SetTitle (BodyProperty::ToString (property));
    colorBarModel->SetInterval (simulation.GetRange (property));
    colorBarModel->SetupPalette (
	Palette (PaletteType::DIVERGING, PaletteDiverging::BLUE_RED));
}



void MainWindow::updateLightControls (
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
    ViewNumber::Enum viewNumber = glWidget->GetViewNumber ();
    size_t simulationIndex = 
	glWidget->GetViewSettings (viewNumber).GetSimulationIndex ();
    ViewSettings& vs = glWidget->GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceProperty ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    return getColorBarModel (simulationIndex, 
			     viewNumber, viewType, property, statisticsType);
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
    boost::array<QWidget*, 4> widgetsVisible = {{
	    labelFacesHistogram, radioButtonHistogramNone,
	    radioButtonHistogramUnicolor, 
	    radioButtonHistogramColorCoded}};
    boost::array<QWidget*, 1> widgetsEnabled = {{
	    radioButtonFacesStatistics}};
    size_t simulationIndex = 
	glWidget->GetViewSettings (viewNumber).GetSimulationIndex ();
    if (value == FaceProperty::DMP_COLOR) {
	::setVisible (widgetsVisible, false);
	::setEnabled (widgetsEnabled, false);
	Q_EMIT BodyOrFacePropertyChanged (
	    viewNumber,
	    boost::shared_ptr<ColorBarModel> (), value);
    }
    else {
	BodyProperty::Enum property = BodyProperty::FromSizeT (value);
	::setVisible (widgetsVisible, true);
	::setEnabled (widgetsEnabled, true);
	Q_EMIT BodyOrFacePropertyChanged (
	    viewNumber,
	    m_colorBarModelBodyProperty
	    [simulationIndex][viewNumber][property], property);
	SetAndDisplayHistogram (DISCARD_SELECTION, REPLACE_MAX_VALUE);
    }
}


// Slots
// ======================================================================

void MainWindow::ToggledVelocityShown (bool checked)
{
    vector<ViewNumber::Enum> vn = glWidget->GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	const ViewSettings& vs = glWidget->GetViewSettings (viewNumber);
	size_t simulationIndex = vs.GetSimulationIndex ();
	boost::shared_ptr<ColorBarModel> overlayBarModel = 
	    m_overlayBarModelVelocityVector[simulationIndex][viewNumber];
	Q_EMIT OverlayBarModelChanged (viewNumber, overlayBarModel);
    }
    glWidget->ToggledVelocityShown (checked);
}

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
    vector<ViewNumber::Enum> vn = glWidget->GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	size_t simulationIndex = 
	    glWidget->GetViewSettings (viewNumber).GetSimulationIndex ();
	m_colorBarModelT1sPDE[simulationIndex][viewNumber]->SetInterval (
	    toQwtDoubleInterval (glWidget->GetRangeT1sPDE (viewNumber)));
    }
}

void MainWindow::ValueChangedSliderTimeSteps (int timeStep)
{
    (void)timeStep;
    SetAndDisplayHistogram (KEEP_SELECTION, KEEP_MAX_VALUE);
    Foam& foam = const_cast<Foam&>
	(glWidget->GetSimulation ().GetFoam (0));
    if (m_debugTranslatedBody)
	m_currentTranslatedBody = foam.GetBodies ().begin ();
    updateButtons ();
}

void MainWindow::ValueChangedStatisticsTimeWindow (int timeSteps)
{
    ViewNumber::Enum viewNumber = glWidget->GetViewNumber ();
    ostringstream ostr;
    ostr << timeSteps * glWidget->LinkedTimeStepStretch (viewNumber);
    labelStatisticsLinkedTimeWindow->setText (ostr.str ().c_str ());
}


void MainWindow::ButtonClickedViewType (int vt)
{
    vector<ViewNumber::Enum> vn = glWidget->GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	const Simulation& simulation = glWidget->GetSimulation (viewNumber);
	ViewType::Enum viewType = ViewType::Enum(vt);
	size_t simulationIndex = 
	    glWidget->GetViewSettings (viewNumber).GetSimulationIndex ();
	ViewSettings& vs = glWidget->GetViewSettings (viewNumber);
	ViewType::Enum oldViewType = vs.GetViewType ();
	size_t property = vs.GetBodyOrFaceProperty ();
	StatisticsType::Enum statisticsType = vs.GetStatisticsType ();

	setStackedWidget (viewType);
	Q_EMIT ColorBarModelChanged (
	    viewNumber,
	    getColorBarModel (simulationIndex, 
			      viewNumber, viewType, property, statisticsType));
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

void MainWindow::CurrentIndexChangedSimulation (int simulationIndex)
{
    ViewNumber::Enum viewNumber = glWidget->GetViewNumber ();
    ViewSettings& vs = glWidget->GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceProperty ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    Q_EMIT ColorBarModelChanged (
	viewNumber,
	getColorBarModel (simulationIndex, 
			  viewNumber, viewType, property, statisticsType));
    ViewToUI ();
}

void MainWindow::CurrentIndexChangedSelectedLight (int i)
{
    const ViewSettings& vs = glWidget->GetViewSettings ();
    LightNumber::Enum lightNumber = LightNumber::Enum (i);
    updateLightControls (vs, lightNumber);
}

void MainWindow::CurrentIndexChangedFaceColor (int value)
{
    (void)value;
    glWidget->SetOneOrTwoViews (this, &MainWindow::currentIndexChangedFaceColor);
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
    Q_EMIT ColorBarModelChanged (glWidget->GetViewNumber (), 
				 getColorBarModel ());
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
	resize (1062, 612);
	break;
    }
}


void MainWindow::ButtonClickedHistogram (int histogramType)
{
    m_histogramType = HistogramType::Enum (histogramType);
    m_histogramViewNumber = glWidget->GetViewNumber ();
    SetAndDisplayHistogram (KEEP_SELECTION, REPLACE_MAX_VALUE);
}



void MainWindow::ToggledReflectedHalfView (bool reflectedHalfView)
{
    if (reflectedHalfView &&
	(glWidget->GetViewCount () != ViewCount::TWO || 
	 glWidget->GetViewLayout () != ViewLayout::VERTICAL))
    {
	QMessageBox msgBox (this);
	msgBox.setText("This feature works only with two views "
		       "in vertical layout");
	msgBox.exec();
	SetCheckedNoSignals (checkBoxReflectedHalfView, false, true);
	return;
    }
    checkBoxTitleShown->setChecked (false);
    glWidget->SetReflectedHalfView (reflectedHalfView);
}

void MainWindow::ToggledForceDifference (bool forceDifference)
{
    const ViewSettings& vs = glWidget->GetViewSettings ();
    if (! vs.IsAverageAround () ||
	vs.GetAverageAroundSecondBodyId () == INVALID_INDEX)
    {
	QMessageBox msgBox (this);
	msgBox.setText("This feature works only when "
		       "averaging around two objects.");
	msgBox.exec();
	SetCheckedNoSignals (checkBoxForceDifference, false, true);
	return;
    }
    glWidget->SetForceDifferenceShown (forceDifference);
}

void MainWindow::SelectionChangedHistogram ()
{
    vector<QwtDoubleInterval> valueIntervals;
    widgetHistogram->GetSelectedIntervals (&valueIntervals);
    vector<bool> timeStepSelection;
    const Simulation& simulation = glWidget->GetSimulation ();
    simulation.GetTimeStepSelection (
	BodyProperty::FromSizeT (
	    glWidget->GetBodyOrFaceProperty (m_histogramViewNumber)), 
	valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);
    
    if (widgetHistogram->AreAllItemsSelected ())
	glWidget->GetViewSettings (m_histogramViewNumber).SetBodySelector (
	    AllBodySelector::Get (), BodySelectorType::PROPERTY_VALUE);
    else
	glWidget->GetViewSettings (m_histogramViewNumber).SetBodySelector (
	    boost::shared_ptr<PropertyValueBodySelector> (
		new PropertyValueBodySelector (
		    BodyProperty::FromSizeT (
			glWidget->GetBodyOrFaceProperty (
			    m_histogramViewNumber)), 
		    valueIntervals)));
    glWidget->update ();
}

void MainWindow::ShowEditOverlayMap ()
{
    HistogramInfo p = getHistogramInfo (
	ColorBarType::PROPERTY, BodyProperty::VELOCITY_MAGNITUDE);
    ViewNumber::Enum viewNumber = glWidget->GetViewNumber ();
    size_t simulationIndex = 
	glWidget->GetViewSettings (viewNumber).GetSimulationIndex ();
    boost::shared_ptr<ColorBarModel> colorBarModel = 
	m_overlayBarModelVelocityVector[simulationIndex][viewNumber];
    m_editColorMap->SetData (p.first, p.second, 
			     *colorBarModel,
			     checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*colorBarModel = m_editColorMap->GetColorBarModel ();
	Q_EMIT OverlayBarModelChanged (glWidget->GetViewNumber (), 
				       colorBarModel);
    }
}


void MainWindow::ShowEditColorMap ()
{
    HistogramInfo p = getHistogramInfo (
	glWidget->GetColorBarType (), glWidget->GetBodyOrFaceProperty ());
    m_editColorMap->SetData (
	p.first, p.second, *getColorBarModel (),
	checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*getColorBarModel () = m_editColorMap->GetColorBarModel ();
	Q_EMIT ColorBarModelChanged (glWidget->GetViewNumber (),
				     getColorBarModel ());
    }
}

void MainWindow::SetHistogramColorBarModel (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<ColorBarModel> colorBarModel)
{
    if (m_histogramViewNumber == viewNumber)
	widgetHistogram->SetColorBarModel (colorBarModel);
}

void MainWindow::CurrentIndexChangedInteractionMode (int index)
{
    InteractionMode::Enum im = InteractionMode::Enum(index);
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

