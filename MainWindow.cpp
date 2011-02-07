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

int checkedId (const QButtonGroup* group)
{
    return group->id (group->checkedButton ());
}

HistogramType::Enum histogramType (const QButtonGroup* group)
{
    return static_cast<HistogramType::Enum> (checkedId (group));
}

MainWindow::MainWindow (FoamAlongTime& foamAlongTime) : 
    PLAY_TEXT (">"), PAUSE_TEXT("||"),
    m_timer (new QTimer(this)), m_processBodyTorus (0), 
    m_currentBody (0),
    m_property (BodyProperty::NONE),
    m_histogramType (HistogramType::NONE),
    m_colorBarModel (BodyProperty::PROPERTY_END),
    m_editColorMap (new EditColorMap (this))
{
    // for anti-aliased lines
    QGLFormat format = QGLFormat::defaultFormat ();
    format.setSampleBuffers (true);
    format.setAlpha (true);
    QGLFormat::setDefaultFormat(format);

    setupUi (this);
    setupSliderData (foamAlongTime);
    setupColorBar ();
    setupHistogram ();
    setupButtonGroups ();

    boost::shared_ptr<Application> app = Application::Get ();
    QFont defaultFont = app->font ();
    spinBoxFontSize->setValue (defaultFont.pointSize ());

    widgetGl->SetFoamAlongTime (&foamAlongTime);
    widgetGl->SetStatus (labelStatusBar);
    setupColorBarModels ();
    widgetHistogram->setHidden (true);
    m_currentTranslatedBody = widgetGl->GetCurrentFoam ().GetBodies ().begin ();
    configureInterface (foamAlongTime);
    

    setWindowTitle (
	QString (
	    (string("Foam - ") + foamAlongTime.GetFilePattern ()).c_str ()));
    // 30 ms
    m_timer->setInterval (30);
    createActions ();

    setTabOrder (radioButtonCenterPath, sliderTimeSteps);

    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));

    connect (widgetGl, SIGNAL (PaintedGL ()),
	     widgetDisplay, SLOT (SaveFrame ()));

    connectColorBarModelChanged ();
}


void MainWindow::connectColorBarModelChanged ()
{
    // from MainWindow to ColorBar, GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (BodyPropertyChanged (
		    boost::shared_ptr<ColorBarModel>,
		    BodyProperty::Enum, ViewType::Enum)),
	colorBar, 
	SLOT (ColorBarModelChangedSlot (
		  boost::shared_ptr<ColorBarModel>)));
    connect (
	this, 
	SIGNAL (BodyPropertyChanged (
		    boost::shared_ptr<ColorBarModel>,
		    BodyProperty::Enum, ViewType::Enum)),
	widgetGl, 
	SLOT (BodyPropertyChanged (
		  boost::shared_ptr<ColorBarModel>,
		  BodyProperty::Enum, ViewType::Enum)));
    connect (
	this, 
	SIGNAL (BodyPropertyChanged (
		    boost::shared_ptr<ColorBarModel>,
		    BodyProperty::Enum, ViewType::Enum)),
	widgetHistogram, 
	SLOT (ColorBarModelChanged (
		  boost::shared_ptr<ColorBarModel>)));

    // from MainWindow to ColorBar, GLWidget and AttributeHistogram
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (
		    boost::shared_ptr<ColorBarModel>)),
	colorBar, 
	SLOT (ColorBarModelChangedSlot (
		  boost::shared_ptr<ColorBarModel>)));
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (
		    boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (ColorBarModelChanged (
		  boost::shared_ptr<ColorBarModel>)));
    connect (
	this, 
	SIGNAL (ColorBarModelChanged (
		    boost::shared_ptr<ColorBarModel>)),
	widgetHistogram, 
	SLOT (ColorBarModelChanged (
		  boost::shared_ptr<ColorBarModel>)));


    // from ColorBar to GLWidget and AttributeHistogram
    connect (
	colorBar, 
	SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	widgetGl, 
	SLOT (ColorBarModelChanged (
		  boost::shared_ptr<ColorBarModel>)));
    connect (
	colorBar, 
	SIGNAL (ColorBarModelChanged (boost::shared_ptr<ColorBarModel>)),
	widgetHistogram, 
	SLOT (ColorBarModelChanged (
		  boost::shared_ptr<ColorBarModel>)));
}


void MainWindow::setupButtonGroups ()
{
    buttonGroupLightPosition->setId (checkBoxTopRight, LightPosition::TOP_RIGHT);
    buttonGroupLightPosition->setId (checkBoxTopLeft, LightPosition::TOP_LEFT);
    buttonGroupLightPosition->setId (checkBoxBottomLeft,
				     LightPosition::BOTTOM_LEFT);
    buttonGroupLightPosition->setId (checkBoxBottomRight,
				     LightPosition::BOTTOM_RIGHT);

    buttonGroupFacesHistogram->setId (
	radioButtonFacesHistogramNone, HistogramType::NONE);
    buttonGroupFacesHistogram->setId (
	radioButtonFacesHistogramUnicolor, HistogramType::UNICOLOR);
    buttonGroupFacesHistogram->setId (
	radioButtonFacesHistogramColorCoded, HistogramType::COLOR_CODED);

    buttonGroupCenterPathHistogram->setId (
	radioButtonCenterPathHistogramNone, HistogramType::NONE);
    buttonGroupCenterPathHistogram->setId (
	radioButtonCenterPathHistogramUnicolor, HistogramType::UNICOLOR);
    buttonGroupCenterPathHistogram->setId (
	radioButtonCenterPathHistogramColorCoded, HistogramType::COLOR_CODED);

    buttonGroupDisplay->setId (radioButtonEdgesNormal, ViewType::EDGES);
    buttonGroupDisplay->setId (radioButtonEdgesTorus, ViewType::EDGES_TORUS);
    buttonGroupDisplay->setId (radioButtonFacesNormal, ViewType::FACES);
    buttonGroupDisplay->setId (radioButtonFaceEdgesTorus, ViewType::FACES_TORUS);
    buttonGroupDisplay->setId (radioButtonFacesAverage, ViewType::FACES_AVERAGE);
    buttonGroupDisplay->setId (radioButtonCenterPath, ViewType::CENTER_PATHS);
}

void MainWindow::setupSliderData (const FoamAlongTime& foamAlongTime)
{
    sliderTimeSteps->setMinimum (0);
    sliderTimeSteps->setMaximum (foamAlongTime.GetTimeSteps () - 1);
    sliderTimeSteps->setSingleStep (1);
    sliderTimeSteps->setPageStep (10);
}

void MainWindow::setupColorBar ()
{
    colorBar->setAlignment (QwtScaleDraw::LeftScale);
    //colorBar->setLabelRotation (-90);
    //colorBar->setLabelAlignment (Qt::AlignHCenter);
    colorBar->setBorderDist (50, 50);
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
    if (! foam->IsTorus ())
    {
	checkBoxTorusOriginalDomain->setDisabled (true);
	checkBoxTorusOriginalDomainWrapInside->setDisabled (true);
	radioButtonEdgesTorus->setDisabled (true);
	radioButtonFaceEdgesTorus->setDisabled (true);
    }
    if (foam->GetDimension () == 2)
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
    colorBar->setHidden (true);
    comboBoxCenterPathColor->setCurrentIndex (BodyProperty::NONE);
    comboBoxFacesColor->setCurrentIndex (BodyProperty::NONE);
}


void MainWindow::InteractionModeRotate ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::ROTATE);
}

void MainWindow::InteractionModeScale ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SCALE);
}

void MainWindow::InteractionModeTranslate ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::TRANSLATE);
}

void MainWindow::InteractionModeSelect ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SELECT);
}

void MainWindow::InteractionModeDeselect ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::DESELECT);
}

void MainWindow::InteractionModeRotateLight ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::ROTATE_LIGHT);
}

void MainWindow::InteractionModeTranslateLight ()
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
    case Qt::Key_Up:
	widgetGl->IncrementSelectedEdgeIndex ();
	break;
    case Qt::Key_Down:
	widgetGl->DecrementSelectedEdgeIndex ();
	break;
    case Qt::Key_PageUp:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->IncrementSelectedBodyIndex ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->IncrementSelectedFaceIndex ();
	    break;
	}
        break;
    case Qt::Key_PageDown:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->DecrementSelectedBodyIndex ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->DecrementSelectedFaceIndex ();
	    break;
	}
	break;
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
    widgetGl->updateGL ();
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
	widgetGl->updateGL ();
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
    m_actionRotateModel = boost::make_shared<QAction> (
	tr("&Rotate Model"), this);
    m_actionRotateModel->setShortcut(QKeySequence (tr ("R")));
    m_actionRotateModel->setStatusTip(tr("Rotate Model"));
    connect(m_actionRotateModel.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeRotate ()));

    m_actionScale = boost::make_shared<QAction> (tr("&Scale"), this);
    m_actionScale->setShortcut(QKeySequence (tr ("Z")));
    m_actionScale->setStatusTip(tr("Scale"));
    connect(m_actionScale.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeScale ()));

    m_actionTranslate = boost::make_shared<QAction> (tr("&Translate"), this);
    m_actionTranslate->setShortcut(QKeySequence (tr ("T")));
    m_actionTranslate->setStatusTip(tr("Translate"));
    connect(m_actionTranslate.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeTranslate ()));


    m_actionRotateLight = boost::make_shared<QAction> (
	tr("Rotate &Light"), this);
    m_actionRotateLight->setShortcut(QKeySequence (tr ("L")));
    m_actionRotateLight->setStatusTip(tr("Rotate Light"));
    connect(m_actionRotateLight.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeRotateLight ()));

    m_actionTranslateLight = boost::make_shared<QAction> (
	tr("Translate L&ight"), this);
    m_actionTranslateLight->setShortcut(QKeySequence (tr ("I")));
    m_actionTranslateLight->setStatusTip(tr("Translate Light"));
    connect(m_actionTranslateLight.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeTranslateLight ()));


    m_actionSelect = boost::make_shared<QAction> (
	tr("&Select"), this);
    m_actionSelect->setShortcut(QKeySequence (tr ("S")));
    m_actionSelect->setStatusTip(tr("Select"));
    connect(m_actionSelect.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeSelect ()));

    m_actionDeselect = boost::make_shared<QAction> (
	tr("&Deselect"), this);
    m_actionDeselect->setShortcut (QKeySequence (tr ("D")));
    m_actionDeselect->setStatusTip (tr("Deselect"));
    connect(m_actionDeselect.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeDeselect ()));

    m_actionInfo = boost::make_shared<QAction> (
	tr("&Info"), this);
    m_actionInfo->setShortcut(
	QKeySequence (tr ("Shift+I")));
    m_actionInfo->setStatusTip(tr("Info"));
    widgetGl->SetActionInfo (m_actionInfo);

    addAction (widgetGl->GetActionResetTransformation ().get ());
    addAction (sliderTimeSteps->GetActionNextSelectedTimeStep ().get ());
    addAction (sliderTimeSteps->GetActionPreviousSelectedTimeStep ().get ());
    addAction (m_actionRotateModel.get ());
    addAction (m_actionTranslate.get ());
    addAction (m_actionScale.get ());
    addAction (m_actionRotateLight.get ());
    addAction (m_actionTranslateLight.get ());
    addAction (m_actionSelect.get ());
    addAction (m_actionDeselect.get ());
    addAction (m_actionInfo.get ());
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

void MainWindow::ToggledColorBarShown (bool checked)
{
    colorBar->setVisible (checked);
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
    colorBar->SetDefaultFont ();    
}

void MainWindow::ValueChangedSliderTimeSteps (int timeStep)
{
    BodyProperty::Enum property = 
	BodyProperty::FromSizeT (comboBoxFacesColor->currentIndex ());
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
	fieldsToControls (comboBoxFacesColor, buttonGroupFacesHistogram);
	ButtonClickedHistogram (m_histogramType);
	stackedWidgetFaces->setCurrentWidget (pageFacesNormal);
	checkBoxShowEdges->setHidden (radioButtonFacesAverage->isChecked ());
	widgetStatisticsType->setVisible (
	    radioButtonFacesAverage->isChecked ());
	/*sliderTimeSteps->setHidden (
	    radioButtonFacesAverage->isChecked () || 
	    ! checkBoxTimeStepsShown->isChecked ());*/
    }
    else
    {
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);

	//sliderTimeSteps->setHidden (!checkBoxTimeStepsShown->isChecked ());
    }
    displayHistogramColorBar (checked);
}

void MainWindow::ToggledCenterPath (bool checked)
{
    if (checked)
    {
	fieldsToControls (comboBoxCenterPathColor,
			  buttonGroupCenterPathHistogram);
	ButtonClickedHistogram (m_histogramType);
	stackedWidgetComposite->setCurrentWidget (pageCenterPath);
    }
    else
	stackedWidgetComposite->setCurrentWidget (pageCompositeEmpty);
    displayHistogramColorBar (checked);
}


void MainWindow::fieldsToControls (QComboBox* comboBox,
				   QButtonGroup* buttonGroup)
{
    BodyProperty::Enum property = 
	BodyProperty::FromSizeT(comboBox->currentIndex ());
    if (property != m_property)
	comboBox->setCurrentIndex (m_property);
    int type = histogramType(buttonGroup);
    if (type != m_histogramType)
	buttonGroup->button (m_histogramType)->setChecked (true);
}

void MainWindow::displayHistogramColorBar (bool checked)
{
    widgetHistogram->setVisible (
	checked && m_property != BodyProperty::NONE && m_histogramType);
    colorBar->setVisible (
	checked && m_property != BodyProperty::NONE);
}

void MainWindow::setupColorBarModels ()
{
    size_t i = 0;
    BOOST_FOREACH (boost::shared_ptr<ColorBarModel>& colorBarModel,
		   m_colorBarModel)
    {
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	colorBarModel.reset (new ColorBarModel ());
	setupColorBarModel (property);
	++i;
    }
}

void MainWindow::setupColorBarModel (BodyProperty::Enum property)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_colorBarModel[property]->SetTitle (
	BodyProperty::ToString (property));
    m_colorBarModel[property]->SetInterval (
	foamAlongTime.GetRange (property));
    m_colorBarModel[property]->SetupPalette (Palette::RAINBOW);
}



void MainWindow::CurrentIndexChangedFacesColor (int value)
{
    BodyProperty::Enum property = BodyProperty::FromSizeT (value);
    m_property = property;
    if (property == BodyProperty::NONE)
    {
	groupBoxFacesHistogram->setHidden (true);
	colorBar->setHidden (true);
	widgetHistogram->setHidden (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[0], property, 
	    ViewType::FromInt (buttonGroupDisplay->checkedId ()));
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	size_t timeStep = widgetGl->GetTimeStep ();
	groupBoxFacesHistogram->setVisible (true);
	colorBar->setVisible (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[property], property, 
	    ViewType::FromInt (buttonGroupDisplay->checkedId ()));
	if (m_histogramType != HistogramType::NONE)
	    SetAndDisplayHistogram (
		m_histogramType,
		property,
		foamAlongTime.GetFoam (timeStep)->
		GetHistogram (property).ToQwtIntervalData (),
		foamAlongTime.GetMaxCountPerBinIndividual (property));
    }
}

void MainWindow::CurrentIndexChangedCenterPathColor (int value)
{
    BodyProperty::Enum property = BodyProperty::FromSizeT(value);
    m_property = property;
    if (property == BodyProperty::NONE)
    {
	groupBoxCenterPathHistogram->setHidden (true);
	colorBar->setHidden (true);
	widgetHistogram->setHidden (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[0], property, ViewType::CENTER_PATHS);
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	groupBoxCenterPathHistogram->setVisible (true);
	colorBar->setVisible (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[property], property, ViewType::CENTER_PATHS);
	if (m_histogramType != HistogramType::NONE)
	    SetAndDisplayHistogram (
		m_histogramType,
		property,
		foamAlongTime.GetHistogram (property).ToQwtIntervalData (),
		foamAlongTime.GetHistogram (property).GetMaxCountPerBin ());
    }
}


void MainWindow::ButtonClickedHistogram (int histogramType)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_histogramType = static_cast<HistogramType::Enum> (histogramType);
    if (m_histogramType == HistogramType::NONE ||
	m_property == BodyProperty::NONE)
    {
	widgetHistogram->setHidden (true);
	widgetHistogram->SetColorCoded (false);
	return;
    }
    if (radioButtonFacesNormal->isChecked ())
	SetAndDisplayHistogram (
	    m_histogramType, m_property,
	    foamAlongTime.GetFoam (widgetGl->GetTimeStep ())->GetHistogram (
		m_property).ToQwtIntervalData (),
	    foamAlongTime.GetMaxCountPerBinIndividual (m_property), 
	    KEEP_SELECTION);
    else
    {
	const HistogramStatistics& histogramStatistics = 
	    foamAlongTime.GetHistogram (m_property);
	SetAndDisplayHistogram (
	    m_histogramType, m_property,
	    histogramStatistics.ToQwtIntervalData (),
	    histogramStatistics.GetMaxCountPerBin (),
	    KEEP_SELECTION);
    }
}

void MainWindow::SelectionChangedHistogram ()
{
    vector<QwtDoubleInterval> valueIntervals;
    widgetHistogram->GetSelectedIntervals (&valueIntervals);
    vector<bool> timeStepSelection;
    const FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    foamAlongTime.GetTimeStepSelection (
	m_property, valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);

    if (widgetHistogram->AreAllItemsSelected ())
	widgetGl->SetBodySelector (
	    AllBodySelector::Get (), BodySelectorType::PROPERTY_VALUE);
    else
	widgetGl->SetBodySelector (
	    boost::shared_ptr<PropertyValueBodySelector> (
		new PropertyValueBodySelector (
		    m_property, valueIntervals, foamAlongTime)));
}

void MainWindow::ShowEditColorMap ()
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    const HistogramStatistics& histogramStatistics = 
	foamAlongTime.GetHistogram (m_property);
    m_editColorMap->SetData (
	histogramStatistics.ToQwtIntervalData (),
	histogramStatistics.GetMaxCountPerBin (),
	*m_colorBarModel[m_property],
	checkBoxHistogramGridShown->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*m_colorBarModel[m_property] = m_editColorMap->GetColorBarModel ();
	Q_EMIT ColorBarModelChanged (m_colorBarModel[m_property]);
    }
}
