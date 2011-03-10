/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "Application.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "Debug.h"
#include "EditColorMap.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ProcessBodyTorus.h"
#include "SystemDifferences.h"
#include "Utils.h"
#include "OpenGLUtils.h"
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


// Methods
// ======================================================================

MainWindow::MainWindow (FoamAlongTime& foamAlongTime) : 
    PLAY_TEXT (">"), PAUSE_TEXT("||"),
    m_timer (new QTimer(this)), m_processBodyTorus (0), 
    m_currentBody (0),
    m_histogramType (HistogramType::NONE),
    m_editColorMap (new EditColorMap (this))
{
    // for anti-aliased lines
    QGLFormat format = QGLFormat::defaultFormat ();
    format.setSampleBuffers (true);
    format.setAlpha (true);
    QGLFormat::setDefaultFormat(format);
    
    setupUi (this);
    CurrentIndexChangedViewCount (ViewCount::ONE);
    setupSliderData (foamAlongTime);
    setupHistogram ();
    setupButtonGroups ();
    
    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    spinBoxFontSize->setValue (defaultFont.pointSize ());
    spinBoxStatisticsHistory->setMaximum (foamAlongTime.GetTimeSteps ());
    spinBoxStatisticsHistory->setValue (spinBoxStatisticsHistory->maximum ());
    widgetGl->SetFoamAlongTime (&foamAlongTime);
    widgetGl->SetStatus (labelStatusBar);
    setupColorBarModels ();
    widgetHistogram->setHidden (true);
    m_currentTranslatedBody = widgetGl->GetCurrentFoam ().GetBodies ().begin ();
    configureInterface (foamAlongTime);    
    setWindowTitle (QString ((string("Foam - ") + 
			      foamAlongTime.GetFilePattern ()).c_str ()));
    m_timer->setInterval (30);    // 30 ms
    createActions ();
    setTabOrder (radioButtonCenterPath, sliderTimeSteps);
    connectSignals ();
}


void MainWindow::connectSignals ()
{
    
    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));
    
    connect (widgetGl, SIGNAL (PaintedGL ()),
	     widgetDisplay, SLOT (SaveFrame ()));
    
    
    // BodyPropertyChanged: 
    // from MainWindow to GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (BodyPropertyChanged (
		    boost::shared_ptr<ColorBarModel>,
		    BodyProperty::Enum)),
	widgetGl, 
	SLOT (SetBodyProperty (
		  boost::shared_ptr<ColorBarModel>,
		  BodyProperty::Enum)));
    connect (
	this, 
	SIGNAL (BodyPropertyChanged (
		    boost::shared_ptr<ColorBarModel>,
		    BodyProperty::Enum)),
	widgetHistogram, 
	SLOT (SetColorBarModel (boost::shared_ptr<ColorBarModel>)));
    
    // ColorBarModelChanged:
    // from MainWindow to ColorBar, GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (SetColorBarModel (boost::shared_ptr<ColorBarModel>)));
    
    
    // ColorBarModelChanged:
    // from ColorBar to GLWidget and AttributeHistogram
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

void MainWindow::ViewToUI ()
{
    boost::shared_ptr<ViewSettings> vs = widgetGl->GetViewSettings ();
    buttonGroupViewType->button (vs->GetViewType ())->setChecked (true);
    comboBoxColor->setCurrentIndex (vs->GetBodyProperty ());
    comboBoxStatisticsType->setCurrentIndex (vs->GetStatisticsType ());
}

void MainWindow::connectColorBarHistogram (bool connected)
{
    if (connected)
    {
	connect (
	    this, 
	    SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	    widgetHistogram, 
	    SLOT (SetColorBarModel (boost::shared_ptr<ColorBarModel>)), 
	    Qt::UniqueConnection);
	connect (
	    widgetGl, 
	    SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	    widgetHistogram, 
	    SLOT (SetColorBarModel (boost::shared_ptr<ColorBarModel>)), 
	    Qt::UniqueConnection);
    }
    else
    {
	disconnect (widgetHistogram);
    }
}


void MainWindow::setupButtonGroups ()
{
    buttonGroupFacesHistogram->setId (
	radioButtonHistogramNone, HistogramType::NONE);
    buttonGroupFacesHistogram->setId (
	radioButtonHistogramUnicolor, HistogramType::UNICOLOR);
    buttonGroupFacesHistogram->setId (
	radioButtonHistogramColorCoded, HistogramType::COLOR_CODED);
        
    buttonGroupViewType->setId (radioButtonEdgesNormal, ViewType::EDGES);
    buttonGroupViewType->setId (radioButtonEdgesTorus, ViewType::EDGES_TORUS);
    buttonGroupViewType->setId (radioButtonFacesNormal, ViewType::FACES);
    buttonGroupViewType->setId (radioButtonFaceEdgesTorus, ViewType::FACES_TORUS);
    buttonGroupViewType->setId (radioButtonFacesStatistics, 
				ViewType::FACES_STATISTICS);
    buttonGroupViewType->setId (radioButtonCenterPath, ViewType::CENTER_PATHS);
}

void MainWindow::setupSliderData (const FoamAlongTime& foamAlongTime)
{
    sliderTimeSteps->setMinimum (0);
    sliderTimeSteps->setMaximum (foamAlongTime.GetTimeSteps () - 1);
    sliderTimeSteps->setSingleStep (1);
    sliderTimeSteps->setPageStep (10);
}

void MainWindow::setupHistogram ()
{
    // does not work
    QwtScaleWidget* yLeftAxis = widgetHistogram->axisWidget (QwtPlot::yLeft);
    yLeftAxis->setBorderDist (100, 100);
}

void MainWindow::configureInterface (const FoamAlongTime& foamAlongTime)
{
    if (foamAlongTime.GetTimeSteps () == 1)
	sliderTimeSteps->setDisabled (true);
    boost::shared_ptr<const Foam> foam = foamAlongTime.GetFoam (0);
    if (foamAlongTime.T1sAvailable ())
	checkBoxT1sShown->setEnabled (true);
    if (! foam->IsTorus ())
    {
	checkBoxTorusOriginalDomain->setDisabled (true);
	checkBoxTorusOriginalDomainWrapInside->setDisabled (true);
	radioButtonEdgesTorus->setDisabled (true);
	radioButtonFaceEdgesTorus->setDisabled (true);
    }
    if (foam->Is2D ())
    {
	radioButtonEdgesNormal->toggle ();
	tabWidget->setCurrentWidget (edges);
	comboBoxAxesOrder->setCurrentIndex (AxesOrder::TWO_D);
	
    }
    else
    {
	radioButtonFacesNormal->toggle ();
	tabWidget->setCurrentWidget (faces);
	comboBoxAxesOrder->setCurrentIndex (AxesOrder::THREE_D);
    }
    comboBoxColor->setCurrentIndex (BodyProperty::NONE);
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

void MainWindow::RotateLightShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::ROTATE_LIGHT);
}

void MainWindow::TranslateLightShown ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::TRANSLATE_LIGHT);
}


void MainWindow::updateButtons ()
{
    enableBegin ();
    enableEnd ();
    enablePlay ();
}

void MainWindow::enableBegin ()
{
    if (sliderTimeSteps->value () > sliderTimeSteps->minimum ())
	toolButtonBegin->setEnabled (true);
}

void MainWindow::enableEnd ()
{
    if (sliderTimeSteps->value () < sliderTimeSteps->maximum ())
	toolButtonEnd->setEnabled (true);
}

void MainWindow::enablePlay ()
{
    if (sliderTimeSteps->value () < sliderTimeSteps->maximum ())
	toolButtonPlay->setEnabled (true);
}


void MainWindow::keyPressEvent (QKeyEvent* event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers ();
    switch (event->key ())
    {
    case Qt::Key_Space:
	translateBodyStep ();
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

void MainWindow::translateBodyStep ()
{
    Foam& currentFoam = widgetGl->GetCurrentFoam ();
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

void MainWindow::processBodyTorusStep ()
{
    try
    {
	const Foam& currentFoam = widgetGl->GetCurrentFoam ();
	boost::shared_ptr<Body>  b = currentFoam.GetBody (m_currentBody);
	if (m_processBodyTorus == 0)
	{
	    m_processBodyTorus = new ProcessBodyTorus (
		currentFoam, b);
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
		    widgetGl->GetCurrentFoam ().GetBodies ().size ();
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
    HistogramType::Enum histogramType,
    BodyProperty::Enum property,
    const QwtIntervalData& intervalData,
    double maxYValue,
    HistogramSelection histogramSelection,
    MaxValueOperation maxValueOperation)
{
    switch (histogramType)
    {
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
    
    
    m_actionRotateLightShown = boost::make_shared<QAction> (
	tr("Rotate &Light"), this);
    m_actionRotateLightShown->setShortcut(QKeySequence (tr ("L")));
    m_actionRotateLightShown->setStatusTip(tr("Rotate Light"));
    connect(m_actionRotateLightShown.get (), SIGNAL(triggered()),
	    this, SLOT(RotateLightShown ()));
    
    m_actionTranslateLightShown = boost::make_shared<QAction> (
	tr("Translate L&ight"), this);
    m_actionTranslateLightShown->setShortcut(QKeySequence (tr ("I")));
    m_actionTranslateLightShown->setStatusTip(tr("Translate Light"));
    connect(m_actionTranslateLightShown.get (), SIGNAL(triggered()),
	    this, SLOT(TranslateLightShown ()));
    
    
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
    
    addAction (widgetGl->GetActionResetTransformation ().get ());
    addAction (widgetGl->GetActionResetSelectedLightPosition ().get ());
    
    addAction (sliderTimeSteps->GetActionNextSelectedTimeStep ().get ());
    addAction (sliderTimeSteps->GetActionPreviousSelectedTimeStep ().get ());
    addAction (m_actionRotateShown.get ());
    addAction (m_actionTranslateShown.get ());
    addAction (m_actionScaleShown.get ());
    addAction (m_actionRotateLightShown.get ());
    addAction (m_actionTranslateLightShown.get ());
    addAction (m_actionSelectShown.get ());
    addAction (m_actionDeselectShown.get ());
}


// Slots
// ======================================================================

void MainWindow::ToggledTimeStepsShown (bool checked)
{
    sliderTimeSteps->setVisible (checked);
}

void MainWindow::ToggledHistogramGridShown (bool checked)
{
    widgetHistogram->SetGridEnabled (checked);
}

void MainWindow::ToggledStatusBarShown (bool checked)
{
    labelStatusBar->setVisible (checked);
}

void MainWindow::ClickedPlay ()
{
    bool playMovie = widgetGl->IsPlayMovie ();
    if (playMovie)
    {
	m_timer->stop ();
	toolButtonPlay->setText (PLAY_TEXT);
	updateButtons ();
    }
    else
    {
	m_timer->start ();
	toolButtonPlay->setText (PAUSE_TEXT);
	toolButtonBegin->setDisabled (true);
	toolButtonEnd->setDisabled (true);
    }
    widgetGl->SetPlayMovie (! playMovie);
}

void MainWindow::ClickedBegin ()
{
    sliderTimeSteps->setValue (sliderTimeSteps->minimum ());
    updateButtons ();
}

void MainWindow::ClickedEnd ()
{
    sliderTimeSteps->setValue (sliderTimeSteps->maximum ());
    updateButtons ();
}

void MainWindow::TimeoutTimer ()
{
    int value = sliderTimeSteps->value ();
    if (value < sliderTimeSteps->maximum ())
	sliderTimeSteps->setValue (value + 1);
    else
	ClickedPlay ();
}

void MainWindow::ValueChangedFontSize (int fontSize)
{
    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    defaultFont.setPointSize (fontSize);
    app->setFont (defaultFont);
    widgetHistogram->SetDefaultFont ();
}

void MainWindow::ValueChangedSliderTimeSteps (int timeStep)
{
    BodyProperty::Enum property = 
	BodyProperty::FromSizeT (comboBoxColor->currentIndex ());
    if (widgetHistogram->isVisible () && 
	radioButtonFacesNormal->isChecked ())
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	
	SetAndDisplayHistogram (
	    m_histogramType,
	    property,
	    foamAlongTime.GetFoam (timeStep)->
	    GetHistogram (property).ToQwtIntervalData (), 
	    0,
	    KEEP_SELECTION,
	    KEEP_MAX_VALUE);
    }
    updateButtons ();
}

void MainWindow::ToggledEdgesNormal (bool checked)
{
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesNormal);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}

void MainWindow::ToggledFacesNormal (bool checked)
{
    if (checked)
    {
	stackedWidgetFaces->setCurrentWidget (pageFacesNormal);
	ButtonClickedOneTimestepHistogram (m_histogramType);
    }
    else
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
}

void MainWindow::ToggledCenterPath (bool checked)
{
    if (checked)
    {
	ButtonClickedAllTimestepsHistogram (m_histogramType);
	stackedWidgetComposite->setCurrentWidget (pageCenterPath);
	labelCenterPathColor->setText (
	    BodyProperty::ToString (
		widgetGl->GetViewSettings ()->GetBodyProperty ()));
    }
    else
	stackedWidgetComposite->setCurrentWidget (pageCompositeEmpty);
}


void MainWindow::displayHistogramColorBar (bool checked)
{
    widgetHistogram->setVisible (
	checked && 
	widgetGl->GetCurrentBodyProperty () != BodyProperty::NONE && 
	m_histogramType);
}

void MainWindow::setupColorBarModels ()
{
    size_t i = 0;
    BOOST_FOREACH (boost::shared_ptr<ColorBarModel>& colorBarModel,
		   m_colorBarModelBodyProperty)
    {
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	colorBarModel.reset (new ColorBarModel ());
	setupColorBarModel (property);
	++i;
    }
    m_colorBarModelDomainHistogram.reset (new ColorBarModel ());
    m_colorBarModelDomainHistogram->SetTitle ("Count per area");
    m_colorBarModelDomainHistogram->SetInterval (
	QwtDoubleInterval (0, widgetGl->GetFoamAlongTime ().GetTimeSteps ()));
    m_colorBarModelDomainHistogram->SetupPalette (Palette::BLACK_BODY);
}

void MainWindow::setupColorBarModel (BodyProperty::Enum property)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_colorBarModelBodyProperty[property]->SetTitle (
	BodyProperty::ToString (property));
    m_colorBarModelBodyProperty[property]->SetInterval (
	foamAlongTime.GetRange (property));
    m_colorBarModelBodyProperty[property]->SetupPalette (Palette::RAINBOW);
}

void MainWindow::CurrentIndexChangedSelectedLight (int i)
{
    checkBoxLightEnabled->setChecked (
	widgetGl->IsLightEnabled (LightPosition::Enum(i)));
    checkBoxLightPositionShown->setChecked (
	widgetGl->IsLightPositionShown (LightPosition::Enum(i)));
    checkBoxDirectionalLightEnabled->setChecked (
	widgetGl->IsDirectionalLightEnabled (
	    LightPosition::Enum(i)));
    
    horizontalSliderLightAmbientRed->setValue (
	floor (
	    widgetGl->GetLightAmbient (LightPosition::Enum(i))[0] * 
	    horizontalSliderLightAmbientRed->maximum () + 0.5));
    horizontalSliderLightAmbientGreen->setValue (
	floor (
	    widgetGl->GetLightAmbient (LightPosition::Enum(i))[1] * 
	    horizontalSliderLightAmbientGreen->maximum () + 0.5));
    horizontalSliderLightAmbientBlue->setValue (
	floor (
	    widgetGl->GetLightAmbient (LightPosition::Enum(i))[2] * 
	    horizontalSliderLightAmbientBlue->maximum () + 0.5));
    horizontalSliderLightDiffuseRed->setValue (
	floor (
	    widgetGl->GetLightDiffuse (LightPosition::Enum(i))[0] * 
	    horizontalSliderLightDiffuseRed->maximum () + 0.5));
    horizontalSliderLightDiffuseGreen->setValue (
	floor (
	    widgetGl->GetLightDiffuse (LightPosition::Enum(i))[1] * 
	    horizontalSliderLightDiffuseGreen->maximum () + 0.5));
    horizontalSliderLightDiffuseBlue->setValue (
	floor (
	    widgetGl->GetLightDiffuse (LightPosition::Enum(i))[2] * 
	    horizontalSliderLightDiffuseBlue->maximum () + 0.5));
    horizontalSliderLightSpecularRed->setValue (
	floor (
	    widgetGl->GetLightSpecular (
		LightPosition::Enum(i))[0] * 
	    horizontalSliderLightSpecularRed->maximum () + 0.5));
    horizontalSliderLightSpecularGreen->setValue (
	floor (
	    widgetGl->GetLightSpecular (
		LightPosition::Enum(i))[1] * 
	    horizontalSliderLightSpecularGreen->maximum () + 0.5));
    horizontalSliderLightSpecularBlue->setValue (
	floor (
	    widgetGl->GetLightSpecular (
		LightPosition::Enum(i))[2] * 
	    horizontalSliderLightSpecularBlue->maximum () + 0.5));
}

void MainWindow::CurrentIndexChangedFacesColor (int value)
{
    boost::array<QWidget*, 4> widgetsVisible = {{
	    labelFacesHistogram,
	    radioButtonHistogramNone,
	    radioButtonHistogramUnicolor,
	    radioButtonHistogramColorCoded}};
    boost::array<QWidget*, 1> widgetsEnabled = {{
	    radioButtonFacesStatistics}};
    BodyProperty::Enum property = BodyProperty::FromSizeT (value);
    if (property == BodyProperty::NONE)
    {
	::setVisible (widgetsVisible, false);
	::setEnabled (widgetsEnabled, false);
	widgetHistogram->setVisible (false);
	Q_EMIT BodyPropertyChanged (m_colorBarModelBodyProperty[0], property);
    }
    else
    {
	::setVisible (widgetsVisible, true);
	::setEnabled (widgetsEnabled, true);
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	size_t timeStep = widgetGl->GetTimeStep ();
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModelBodyProperty[property], property);
	if (m_histogramType != HistogramType::NONE)
	    SetAndDisplayHistogram (
		m_histogramType,
		property,
		foamAlongTime.GetFoam (timeStep)->
		GetHistogram (property).ToQwtIntervalData (),
		foamAlongTime.GetMaxCountPerBinIndividual (property),
		DISCARD_SELECTION, REPLACE_MAX_VALUE);
    }
}

void MainWindow::CurrentIndexChangedCenterPathColor (int value)
{
    boost::array<QWidget*, 1> widgetsEnabled = {{
	    radioButtonFacesStatistics}};
    BodyProperty::Enum property = BodyProperty::FromSizeT(value);
    if (property == BodyProperty::NONE)
    {
	::setEnabled (widgetsEnabled, false);
	widgetHistogram->setHidden (true);
	Q_EMIT BodyPropertyChanged (m_colorBarModelBodyProperty[0], property);
    }
    else
    {
	::setEnabled (widgetsEnabled, true);
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModelBodyProperty[property], property);
	if (m_histogramType != HistogramType::NONE)
	    SetAndDisplayHistogram (
		m_histogramType,
		property,
		foamAlongTime.GetHistogram (property).ToQwtIntervalData (),
		foamAlongTime.GetHistogram (property).GetMaxCountPerBin (),
		DISCARD_SELECTION, REPLACE_MAX_VALUE);
    }
}


void MainWindow::ToggledFacesStatistics (bool checked)
{
    if (checked)
    {
	Q_EMIT ColorBarModelChanged (getCurrentColorBarModel ());
	stackedWidgetFaces->setCurrentWidget (pageFacesStatistics);
	labelFacesStatisticsColor->setText (
	    BodyProperty::ToString (
		widgetGl->GetViewSettings ()->GetBodyProperty ()));
	ButtonClickedAllTimestepsHistogram (m_histogramType);
    }
    else
    {	
	Q_EMIT ColorBarModelChanged (
	    m_colorBarModelBodyProperty[widgetGl->GetCurrentBodyProperty ()]);
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
    }
}

void MainWindow::CurrentIndexChangedViewCount (int index)
{
    boost::array<QWidget*, 2> widgetsViewLayout = 
	{{labelViewLayout, comboBoxViewLayout}};
    ViewCount::Enum viewCount = ViewCount::Enum (index);
    if (viewCount == ViewCount::TWO)
	::setVisible (widgetsViewLayout, true);
    else
	::setVisible (widgetsViewLayout, false);
}


void MainWindow::CurrentIndexChangedStatisticsType (int value)
{
    boost::array<QWidget*, 2> widgetsStatisticsHistory = 
	{{ spinBoxStatisticsHistory, labelStatisticsHistory}};
    switch (value)
    {
    case StatisticsType::AVERAGE:
	connectColorBarHistogram (true);
	::setVisible (widgetsStatisticsHistory, true);
	break;
    case StatisticsType::MIN:
    case StatisticsType::MAX:
	connectColorBarHistogram (true);
	::setVisible (widgetsStatisticsHistory, false);
	break;
    case StatisticsType::COUNT:
	connectColorBarHistogram (false);
	::setVisible (widgetsStatisticsHistory, false);
	break;
    }
    Q_EMIT ColorBarModelChanged (getCurrentColorBarModel ());
}

bool MainWindow::isHistogramHidden (HistogramType::Enum histogramType)
{
    if (histogramType == HistogramType::NONE ||
	widgetGl->GetCurrentBodyProperty () == BodyProperty::NONE)
    {
	widgetHistogram->setHidden (true);
	widgetHistogram->SetColorCoded (false);
	return true;
    }
    else
	return false;
}

void MainWindow::ButtonClickedAllTimestepsHistogram (int histogramType)
{
    m_histogramType = HistogramType::Enum (histogramType);
    if (isHistogramHidden (m_histogramType))
	return;
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    const HistogramStatistics& allTimestepsHistogram = 
	foamAlongTime.GetHistogram (widgetGl->GetCurrentBodyProperty ());
    SetAndDisplayHistogram (
	m_histogramType, widgetGl->GetCurrentBodyProperty (),
	allTimestepsHistogram.ToQwtIntervalData (),
	allTimestepsHistogram.GetMaxCountPerBin (),
	KEEP_SELECTION, REPLACE_MAX_VALUE);
}

void MainWindow::ButtonClickedOneTimestepHistogram (int histogramType)
{
    m_histogramType = HistogramType::Enum (histogramType);
    if (isHistogramHidden (m_histogramType))
	return;
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    SetAndDisplayHistogram (
	m_histogramType, widgetGl->GetCurrentBodyProperty (),
	foamAlongTime.GetFoam (widgetGl->GetTimeStep ())->GetHistogram (
	    widgetGl->GetCurrentBodyProperty ()).ToQwtIntervalData (),
	foamAlongTime.GetMaxCountPerBinIndividual (
	    widgetGl->GetCurrentBodyProperty ()), 
	KEEP_SELECTION, REPLACE_MAX_VALUE);
}

void MainWindow::SelectionChangedHistogram ()
{
    vector<QwtDoubleInterval> valueIntervals;
    widgetHistogram->GetSelectedIntervals (&valueIntervals);
    vector<bool> timeStepSelection;
    const FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    foamAlongTime.GetTimeStepSelection (
	widgetGl->GetCurrentBodyProperty (), 
	valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);
    
    if (widgetHistogram->AreAllItemsSelected ())
	widgetGl->SetBodySelector (
	    AllBodySelector::Get (), BodySelectorType::PROPERTY_VALUE);
    else
	widgetGl->SetBodySelector (
	    boost::shared_ptr<PropertyValueBodySelector> (
		new PropertyValueBodySelector (
		    widgetGl->GetCurrentBodyProperty (), valueIntervals)));
}

void MainWindow::ShowEditColorMap ()
{
    HistogramInfo p = getCurrentHistogramInfo ();
    m_editColorMap->SetData (
	p.first, p.second, *getCurrentColorBarModel (),
	checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*getCurrentColorBarModel () = m_editColorMap->GetColorBarModel ();
	Q_EMIT ColorBarModelChanged (getCurrentColorBarModel ());
    }
}

boost::shared_ptr<ColorBarModel> MainWindow::getCurrentColorBarModel () const
{
    if (widgetGl->GetViewSettings ()->GetStatisticsType () == 
	StatisticsType::COUNT)
	return m_colorBarModelDomainHistogram;
    else
	return m_colorBarModelBodyProperty[widgetGl->GetCurrentBodyProperty ()];
}

/**
 * @todo Calculate the correct histogram for the 'domain histogram' image.
 */
MainWindow::HistogramInfo MainWindow::getCurrentHistogramInfo () const
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    if (widgetGl->GetViewSettings ()->GetStatisticsType () == 
	StatisticsType::COUNT)
    {
	size_t fakeHistogramValue = 1;
	QwtArray<QwtDoubleInterval> a (HISTOGRAM_INTERVALS);
	QwtArray<double> d (HISTOGRAM_INTERVALS);
	size_t max = foamAlongTime.GetTimeSteps ();
	double intervalSize = double (max) / HISTOGRAM_INTERVALS;
	for (int i = 0; i < a.size (); ++i)
	{
	    a[i] = QwtDoubleInterval (intervalSize * i, intervalSize* (i+1));
	    d[i] = fakeHistogramValue;
	}
	return HistogramInfo (QwtIntervalData (a, d), fakeHistogramValue);
    }
    else
    {
	const HistogramStatistics& histogramStatistics = 
	    foamAlongTime.GetHistogram (widgetGl->GetCurrentBodyProperty ());
	return HistogramInfo (histogramStatistics.ToQwtIntervalData (), 
			      histogramStatistics.GetMaxCountPerBin ());
    }
}
