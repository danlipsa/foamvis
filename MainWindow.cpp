/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "Application.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
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
    m_bodyProperty (BodyProperty::NONE),
    m_histogramType (HistogramType::NONE),
    m_colorBarModel (BodyProperty::COUNT),
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
	comboBoxViewportTransform->setCurrentIndex (
	    ViewportTransformType::FILL_SCREEN);
	comboBoxAxesOrder->setCurrentIndex (AxesOrder::TWO_D);
	
    }
    else
    {
	radioButtonFacesNormal->toggle ();
	tabWidget->setCurrentWidget (faces);
	widgetBlend->setDisabled (true);
	comboBoxViewportTransform->setCurrentIndex (
	    ViewportTransformType::ALLOW_ROTATION);
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

void MainWindow::InteractionModeSelectBrush ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SELECT_BRUSH);
}

void MainWindow::InteractionModeSelectEraser ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SELECT_ERASER);
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
	widgetGl->IncrementDisplayedEdge ();
	break;
    case Qt::Key_Down:
	widgetGl->DecrementDisplayedEdge ();
	break;
    case Qt::Key_PageUp:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->IncrementDisplayedBody ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->IncrementDisplayedFace ();
	    break;
	}
        break;
    case Qt::Key_PageDown:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->DecrementDisplayedBody ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->DecrementDisplayedFace ();
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
    BodyProperty::Enum bodyProperty,
    const QwtIntervalData& intervalData,
    double maxYValue,
    HistogramSelection histogramSelection,
    MaxValueOperation maxValueOperation)
{
    switch (histogramType)
    {
    case HistogramType::NONE:
	widgetHistogram->setHidden (true);
	widgetHistogram->SetColorCoded (false);
	return;
    case HistogramType::UNICOLOR:
	widgetHistogram->setVisible (true);
	widgetHistogram->SetColorCoded (false);
	break;
    case HistogramType::COLOR_CODED:
	widgetHistogram->setVisible (true);
	widgetHistogram->SetColorCoded (true);
	break;
    }

    if (maxValueOperation == KEEP_MAX_VALUE)
	maxYValue = widgetHistogram->GetMaxValueAxis ();
    if (histogramSelection == KEEP_SELECTION)
	widgetHistogram->SetDataKeepBinSelection (
	    intervalData, maxYValue, BodyProperty::ToString (bodyProperty));
    else
	widgetHistogram->SetDataAllBinsSelected (
	    intervalData, maxYValue, BodyProperty::ToString (bodyProperty));
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


    m_actionSelectBrush = boost::make_shared<QAction> (
	tr("&Select Brush"), this);
    m_actionSelectBrush->setShortcut(QKeySequence (tr ("B")));
    m_actionSelectBrush->setStatusTip(tr("Select Brush"));
    connect(m_actionSelectBrush.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeSelectBrush ()));

    m_actionSelectEraser = boost::make_shared<QAction> (
	tr("&Select Eraser"), this);
    m_actionSelectEraser->setShortcut (QKeySequence (tr ("E")));
    m_actionSelectEraser->setStatusTip (tr("Select Eraser"));
    connect(m_actionSelectEraser.get (), SIGNAL(triggered()),
	    this, SLOT(InteractionModeSelectEraser ()));

    connect (actionOpenGL_Info, SIGNAL (triggered ()),
	     widgetGl, SLOT (ShowOpenGLInfo ()));


    m_actionSelectAll = boost::make_shared<QAction> (tr("&Select All"), this);
    m_actionSelectAll->setShortcut(
	QKeySequence (tr ("Shift+S")));
    m_actionSelectAll->setStatusTip(tr("Select All"));
    widgetHistogram->SetActionSelectAll (m_actionSelectAll);
    widgetGl->SetActionSelectAll (m_actionSelectAll);

    m_actionDeselectAll = boost::make_shared<QAction> (
	tr("&Deselect All"), this);
    m_actionDeselectAll->setShortcut(
	QKeySequence (tr ("Shift+D")));
    m_actionDeselectAll->setStatusTip(tr("Deselect All"));
    widgetHistogram->SetActionDeselectAll (m_actionDeselectAll);
    widgetGl->SetActionDeselectAll (m_actionDeselectAll);

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
    addAction (m_actionSelectBrush.get ());
    addAction (m_actionSelectEraser.get ());
    addAction (m_actionInfo.get ());
}


// Slots
// ======================================================================

void MainWindow::ToggledShowTimeSteps (bool checked)
{
    sliderTimeSteps->setVisible (checked);
}

void MainWindow::ToggledHistogramGrid (bool checked)
{
    widgetHistogram->SetGridEnabled (checked);
}

void MainWindow::ToggledFullColorBarShown (bool checked)
{
    colorBar->setVisible (checked);
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
    BodyProperty::Enum bodyProperty = 
	BodyProperty::FromSizeT (comboBoxFacesColor->currentIndex ());
    if (widgetHistogram->isVisible () && 
	radioButtonFacesNormal->isChecked ())
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	
	SetAndDisplayHistogram (histogramType (buttonGroupFacesHistogram),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty, timeStep), 
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
	    ! checkBoxTimeSteps->isChecked ());*/
    }
    else
    {
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);

	//sliderTimeSteps->setHidden (!checkBoxTimeSteps->isChecked ());
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
    BodyProperty::Enum bodyProperty = 
	BodyProperty::FromSizeT(comboBox->currentIndex ());
    if (bodyProperty != m_bodyProperty)
	comboBox->setCurrentIndex (m_bodyProperty);
    int type = histogramType(buttonGroup);
    if (type != m_histogramType)
	buttonGroup->button (m_histogramType)->setChecked (true);
}

void MainWindow::displayHistogramColorBar (bool checked)
{
    widgetHistogram->setVisible (
	checked && m_bodyProperty != BodyProperty::NONE && m_histogramType);
    colorBar->setVisible (
	checked && m_bodyProperty != BodyProperty::NONE);
}

void MainWindow::setupColorBarModels ()
{
    size_t i = 0;
    BOOST_FOREACH (boost::shared_ptr<ColorBarModel>& colorBarModel,
		   m_colorBarModel)
    {
	BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (i);
	colorBarModel.reset (new ColorBarModel ());
	setupColorBarModel (bodyProperty);
	++i;
    }
}

void MainWindow::setupColorBarModel (BodyProperty::Enum bodyProperty)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_colorBarModel[bodyProperty]->SetTitle (
	BodyProperty::ToString (bodyProperty));
    m_colorBarModel[bodyProperty]->SetInterval (
	foamAlongTime.GetRange (bodyProperty));
    m_colorBarModel[bodyProperty]->SetupPalette (Palette::RAINBOW);
}



void MainWindow::CurrentIndexChangedFacesColor (int value)
{
    BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (value);
    m_bodyProperty = bodyProperty;
    if (bodyProperty == BodyProperty::NONE)
    {
	groupBoxFacesHistogram->setHidden (true);
	colorBar->setHidden (true);
	widgetHistogram->setHidden (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[0], bodyProperty, 
	    ViewType::FromInt (buttonGroupDisplay->checkedId ()));
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	size_t timeStep = widgetGl->GetTimeStep ();
	groupBoxFacesHistogram->setVisible (true);
	colorBar->setVisible (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[bodyProperty], bodyProperty, 
	    ViewType::FromInt (buttonGroupDisplay->checkedId ()));
	SetAndDisplayHistogram (
	    histogramType (buttonGroupFacesHistogram),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty, timeStep),
	    foamAlongTime.GetMaxCountPerBinIndividual (bodyProperty));
    }
}

void MainWindow::CurrentIndexChangedCenterPathColor (int value)
{
    BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT(value);
    m_bodyProperty = bodyProperty;
    if (bodyProperty == BodyProperty::NONE)
    {
	groupBoxCenterPathHistogram->setHidden (true);
	colorBar->setHidden (true);
	widgetHistogram->setHidden (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[0], bodyProperty, ViewType::CENTER_PATHS);
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	groupBoxCenterPathHistogram->setVisible (true);
	colorBar->setVisible (true);
	Q_EMIT BodyPropertyChanged (
	    m_colorBarModel[bodyProperty], bodyProperty, ViewType::CENTER_PATHS);
	SetAndDisplayHistogram (
	    histogramType (buttonGroupCenterPathHistogram),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty),
	    foamAlongTime.GetMaxCountPerBin (bodyProperty));
    }
}


void MainWindow::ButtonClickedHistogram (int histogramType)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_histogramType = static_cast<HistogramType::Enum> (histogramType);
    if (radioButtonFacesNormal->isChecked ())
	SetAndDisplayHistogram (
	    m_histogramType, m_bodyProperty,
	    foamAlongTime.GetHistogram (
		m_bodyProperty, widgetGl->GetTimeStep ()),
	    foamAlongTime.GetMaxCountPerBinIndividual (m_bodyProperty), 
	    KEEP_SELECTION);
    else
	SetAndDisplayHistogram (
	    m_histogramType, m_bodyProperty,
	    foamAlongTime.GetHistogram (m_bodyProperty),
	    foamAlongTime.GetMaxCountPerBin (m_bodyProperty),
	    KEEP_SELECTION);
}

void MainWindow::SelectionChangedHistogram ()
{
    vector<QwtDoubleInterval> valueIntervals;
    widgetHistogram->GetSelectedIntervals (&valueIntervals);
    vector<bool> timeStepSelection;
    const FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    foamAlongTime.GetTimeStepSelection (
	m_bodyProperty, valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);

    boost::shared_ptr<BodySelector> bodySelector;
    if (widgetHistogram->AreAllItemsSelected ())
	bodySelector = boost::shared_ptr<BodySelector> (
	    new CycleSelector (*widgetGl));
    else
	bodySelector = boost::shared_ptr<BodySelector> (
	    new PropertyValueSelector (
		m_bodyProperty, valueIntervals, foamAlongTime));
    widgetGl->SetBodySelector (bodySelector);
}

void MainWindow::ShowEditColorMap ()
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();    
    m_editColorMap->SetData (
	foamAlongTime.GetHistogram (m_bodyProperty),
	foamAlongTime.GetMaxCountPerBin (m_bodyProperty),
	*m_colorBarModel[m_bodyProperty],
	checkBoxGridEnabled->isChecked ());
    if (m_editColorMap->exec () == QDialog::Accepted)
    {
	*m_colorBarModel[m_bodyProperty] = m_editColorMap->GetColorBarModel ();
	Q_EMIT ColorBarModelChanged (m_colorBarModel[m_bodyProperty]);
    }
}
