/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "ColorBarModel.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ProcessBodyTorus.h"
#include "SystemDifferences.h"
#include "Utils.h"

MainWindow::MainWindow (FoamAlongTime& foamAlongTime) : 
    m_play (false), PLAY_TEXT (">"), PAUSE_TEXT("||"),
    m_timer (new QTimer(this)), m_processBodyTorus (0), 
    m_currentBody (0),
    m_bodyProperty (BodyProperty::NONE),
    m_histogram (false)
{
    // for anti-aliased lines
    QGLFormat fmt;
    fmt.setAlpha(true);
    QGLFormat::setDefaultFormat(fmt);    

    setupUi (this);
    setupSliderData (foamAlongTime);
    setupColorBar ();
    setupHistogram ();

    widgetGl->SetFoamAlongTime (&foamAlongTime);
    boost::shared_ptr<ColorBarModel> colorBarModel;
    widgetGl->SetColorBarModel (colorBarModel);
    widgetHistogram->setHidden (true);
    m_currentTranslatedBody = widgetGl->GetCurrentFoam ().GetBodies ().begin ();
    configureInterface (foamAlongTime);

    setWindowTitle (
	QString (
	    (string("Foam - ") + foamAlongTime.GetFilePattern ()).c_str ()));
    // 30 ms
    m_timer->setInterval (30);
    m_colorBarModel = boost::make_shared<ColorBarModel> ();
    createActions ();


    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));
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
	groupBoxTorusOriginalDomain->setDisabled (true);
    if (foam->GetSpaceDimension () == 2)
    {
	radioButtonEdgesNormal->toggle ();
	tabWidget->setCurrentWidget (edges);
	widgetAngleOfView->setDisabled (true);
    }
    else
    {
	radioButtonFacesNormal->toggle ();
	tabWidget->setCurrentWidget (faces);
    }
    colorBar->setHidden (true);
    comboBoxCenterPathColor->setCurrentIndex (BodyProperty::NONE);
    comboBoxFacesColor->setCurrentIndex (BodyProperty::NONE);
}


void MainWindow::InteractionModeRotate ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::ROTATE);
}

void MainWindow::InteractionModeSelectBrush ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SELECT_BRUSH);
}

void MainWindow::InteractionModeSelectEraser ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SELECT_ERASER);
}


void MainWindow::InteractionModeScale ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::SCALE);
}

void MainWindow::InteractionModeTranslate ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::TRANSLATE);
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
    widgetGl->UpdateDisplayList ();
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
	widgetGl->UpdateDisplayList ();
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
    }
}


// Slots
// ======================================================================

void MainWindow::ClickedPlay ()
{
    if (m_play)
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
    m_play = ! m_play;
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


void MainWindow::ValueChangedSliderTimeSteps (int timeStep)
{
    BodyProperty::Enum bodyProperty = 
	BodyProperty::FromSizeT (comboBoxFacesColor->currentIndex ());
    if (widgetHistogram->isVisible () && 
	radioButtonFacesNormal->isChecked ())
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	
	SetAndDisplayHistogram (
	    checkBoxFacesHistogram->isChecked (),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty, timeStep), 
	    0,
	    KEEP_SELECTION,
	    KEEP_MAX_VALUE);
    }
    widgetGl->ValueChangedSliderTimeSteps (timeStep);
    updateButtons ();
}

void MainWindow::ToggledEdgesNormal (bool checked)
{
    widgetGl->ToggledEdgesNormal (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesNormal);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}


void MainWindow::ToggledEdgesTorus (bool checked)
{
    widgetGl->ToggledEdgesTorus (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesTorus);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}

void MainWindow::ToggledFacesTorus (bool checked)
{
    widgetGl->ToggledFacesTorus (checked);
    if (checked)
	stackedWidgetFaces->setCurrentWidget (pageFacesTorus);
    else
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
}

void MainWindow::fieldsToControls (QComboBox* comboBox, QCheckBox* checkBox)
{
    BodyProperty::Enum bodyProperty = 
	BodyProperty::FromSizeT(comboBox->currentIndex ());
    if (bodyProperty != m_bodyProperty)
	comboBox->setCurrentIndex (m_bodyProperty);
    bool histogram = checkBox->isChecked ();
    if (histogram != m_histogram)
	checkBox->setChecked (m_histogram);    
}

void MainWindow::displayHistogramColorBar (bool checked)
{
    widgetHistogram->setVisible (
	checked && m_bodyProperty != BodyProperty::NONE && m_histogram);
    colorBar->setVisible (
	checked && m_bodyProperty != BodyProperty::NONE);
}


void MainWindow::ToggledCenterPath (bool checked)
{
    widgetGl->ToggledCenterPath (checked);
    if (checked)
    {
	fieldsToControls (comboBoxCenterPathColor, checkBoxCenterPathHistogram);
	ToggledCenterPathHistogram (m_histogram);
	stackedWidgetComposite->setCurrentWidget (pageCenterPath);
    }
    else
	stackedWidgetComposite->setCurrentWidget (pageCompositeEmpty);
    displayHistogramColorBar (checked);
}

void MainWindow::ToggledFacesNormal (bool checked)
{
    widgetGl->ToggledFacesNormal (checked);
    if (checked)
    {
	fieldsToControls (comboBoxFacesColor, checkBoxFacesHistogram);
	ToggledFacesHistogram (m_histogram);
	stackedWidgetFaces->setCurrentWidget (pageFacesNormal);
    }
    else
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
    displayHistogramColorBar (checked);
}

void MainWindow::changedColorBarInterval (BodyProperty::Enum bodyProperty)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_colorBarModel->SetTitle (BodyProperty::ToString (bodyProperty));
    m_colorBarModel->SetInterval (foamAlongTime.GetRange (bodyProperty));
    m_colorBarModel->SetupRainbowColorMap ();
    colorBar->SetModel (m_colorBarModel);
}

void MainWindow::CurrentIndexChangedFacesColor (int value)
{
    BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (value);
    m_bodyProperty = bodyProperty;
    if (bodyProperty == BodyProperty::NONE)
    {
	widgetGl->CurrentIndexChangedFacesColor (bodyProperty);
	boost::shared_ptr<ColorBarModel> colorBarModel;
	widgetGl->SetColorBarModel (colorBarModel);
	checkBoxFacesHistogram->setHidden (true);
	colorBar->setHidden (true);
	widgetHistogram->setHidden (true);
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	size_t timeStep = widgetGl->GetTimeStep ();
	checkBoxFacesHistogram->setVisible (true);
	colorBar->setVisible (true);
	widgetGl->CurrentIndexChangedFacesColor (bodyProperty);
	widgetGl->SetColorBarModel (m_colorBarModel);
	changedColorBarInterval (bodyProperty);
	SetAndDisplayHistogram (
	    checkBoxFacesHistogram->isChecked (),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty, timeStep),
	    foamAlongTime.GetMaxCountPerBinIndividual (bodyProperty));
    }
    widgetGl->UpdateDisplayList ();
}

void MainWindow::CurrentIndexChangedCenterPathColor (int value)
{
    BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT(value);
    m_bodyProperty = bodyProperty;
    if (bodyProperty == BodyProperty::NONE)
    {
	boost::shared_ptr<ColorBarModel> colorBarModel;
	widgetGl->SetColorBarModel (colorBarModel);
	checkBoxCenterPathHistogram->setHidden (true);
	colorBar->setHidden (true);
	widgetHistogram->setHidden (true);
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (
	    comboBoxCenterPathColor->currentIndex ());
	checkBoxCenterPathHistogram->setVisible (true);
	colorBar->setVisible (true);

	widgetGl->SetColorBarModel (m_colorBarModel);
	changedColorBarInterval (bodyProperty);
	widgetGl->CurrentIndexChangedCenterPathColor (bodyProperty);
	SetAndDisplayHistogram (
	    checkBoxCenterPathHistogram->isChecked (),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty),
	    foamAlongTime.GetMaxCountPerBinIndividual (bodyProperty));
    }
    widgetGl->UpdateDisplayList ();
}


void MainWindow::ToggledCenterPathHistogram (bool checked)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_histogram = checked;
    SetAndDisplayHistogram (
	checked, m_bodyProperty,
	foamAlongTime.GetHistogram (m_bodyProperty),
	foamAlongTime.GetMaxCountPerBin (m_bodyProperty));
}

void MainWindow::ToggledFacesHistogram (bool checked)
{
    FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_histogram = checked;
    SetAndDisplayHistogram (
	checked, m_bodyProperty,
	foamAlongTime.GetHistogram (
	    m_bodyProperty, widgetGl->GetTimeStep ()),
	foamAlongTime.GetMaxCountPerBinIndividual (m_bodyProperty));
}

void MainWindow::SetAndDisplayHistogram (
    bool checked,
    BodyProperty::Enum bodyProperty,
    const QwtIntervalData& intervalData,
    double maxYValue,
    HistogramSelection histogramSelection,
    MaxValueOperation maxValueOperation)
{
    if (! checked)
    {
	widgetHistogram->setHidden (true);
	return;
    }

    widgetHistogram->setVisible (true);
    if (maxValueOperation == KEEP_MAX_VALUE)
	maxYValue = widgetHistogram->GetMaxValueAxis ();
    if (histogramSelection == KEEP_SELECTION)
    {
	vector< pair<size_t, size_t> > selectedBins;
	widgetHistogram->GetSelectedBins (&selectedBins);
	widgetHistogram->SetData (intervalData, maxYValue, &selectedBins);
    }
    else
	widgetHistogram->SetData (intervalData, maxYValue);
    widgetHistogram->setAxisTitle (
	QwtPlot::xBottom, QString(BodyProperty::ToString (bodyProperty)));
    widgetHistogram->setAxisTitle (
	QwtPlot::yLeft, QString("Number of values per bin"));
    widgetHistogram->replot ();
}

void MainWindow::createActions ()
{
    m_actionRotate = boost::make_shared<QAction> (tr("&Rotate"), this);
    m_actionRotate->setShortcut(QKeySequence (tr ("R")));
    m_actionRotate->setStatusTip(tr("Rotate"));
    connect(m_actionRotate.get (), SIGNAL(triggered()),
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
    addAction (m_actionRotate.get ());
    addAction (m_actionTranslate.get ());
    addAction (m_actionScale.get ());
    addAction (m_actionSelectBrush.get ());
    addAction (m_actionSelectEraser.get ());
    addAction (m_actionSelectAll.get ());
    addAction (m_actionDeselectAll.get ());
    addAction (m_actionInfo.get ());
}


void MainWindow::SelectionChangedHistogram ()
{
    BodyProperty::Enum bodyProperty = 
	BodyProperty::FromSizeT (comboBoxFacesColor->currentIndex ());
    vector<QwtDoubleInterval> valueIntervals;
    widgetHistogram->GetSelectedIntervals (&valueIntervals);

    vector<bool> timeStepSelection;
    widgetGl->GetFoamAlongTime ().GetTimeStepSelection (
	bodyProperty, valueIntervals, &timeStepSelection);
    sliderTimeSteps->SetRestrictedTo (timeStepSelection);
    cdbg << "SelectionChangedHistogram" << endl;
}
