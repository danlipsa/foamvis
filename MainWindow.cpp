/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
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
    setupScaleWidget ();
    setupHistogram ();

    widgetGl->SetFoamAlongTime (&foamAlongTime);
    widgetGl->SetColorMap (0, 0);
    widgetHistogram->setVisible (false);
    updateStatus ();
    m_currentTranslatedBody = widgetGl->GetCurrentFoam ().GetBodies ().begin ();
    configureInterface (foamAlongTime);

    setWindowTitle (
	QString (
	    (string("Foam - ") + foamAlongTime.GetFilePattern ()).c_str ()));
    // 30 ms
    m_timer->setInterval (30);
    createActions ();


    connect (m_timer.get (), SIGNAL (timeout()),
	     this, SLOT (TimeoutTimer ()));
    connect (widgetHistogram, SIGNAL(selectionChanged ()),
	     this, SLOT (SelectionChangedHistogram ()));
}

void MainWindow::setupSliderData (const FoamAlongTime& foamAlongTime)
{
    sliderData->setMinimum (0);
    sliderData->setMaximum (foamAlongTime.GetTimeSteps () - 1);
    sliderData->setSingleStep (1);
    sliderData->setPageStep (10);
}

void testColorMap ()
{
    cdbg << "Test Color map:" << endl;
    for (size_t i = 0; i <= 12; i++)
    {
	double value = static_cast<double>(i) / 2;
	cdbg << setw (3) << value << " " << RainbowColor (value/6) << endl;
    }
}


void MainWindow::setupRainbowColorMap (QwtLinearColorMap* colorMap)
{
    //testColorMap ();
    const size_t COLORS = 256;
    colorMap->setColorInterval (RainbowColor (0), RainbowColor (1));
    colorMap->setMode (QwtLinearColorMap::FixedColors);
    for (size_t i = 1; i < COLORS; ++i)
    {
	double value = static_cast<double>(i)/COLORS;
	colorMap->addColorStop (value, 	RainbowColor (value));
    }
}

void MainWindow::setupBlueRedColorMap (QwtLinearColorMap* colorMap)
{
    const size_t COLORS = 256;
    colorMap->setColorInterval (BlueRedColor (0), BlueRedColor (COLORS));
    colorMap->setMode (QwtLinearColorMap::FixedColors);
    for (size_t i = 1; i < COLORS; ++i)
    {
	double value = static_cast<double>(i)/COLORS;
	colorMap->addColorStop (value, 	BlueRedColor (i));
    }
}


void MainWindow::setupScaleWidget ()
{
    scaleWidgetColorBar->setAlignment (QwtScaleDraw::RightScale);
    scaleWidgetColorBar->setLabelRotation (-90);
    scaleWidgetColorBar->setLabelAlignment (Qt::AlignHCenter);
    scaleWidgetColorBar->setBorderDist (50, 50);
    scaleWidgetColorBar->setColorBarEnabled (true);
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
	groupBoxTimeSteps->setDisabled (true);
    boost::shared_ptr<const Foam> foam = foamAlongTime.GetFoam (0);
    if (! foam->IsTorus ())
	groupBoxTorusOriginalDomain->setDisabled (true);
    if (foam->GetSpaceDimension () == 2)
    {
	radioButtonEdgesNormal->toggle ();
	tabWidget->setCurrentWidget (edges);
	widgetAngleOfView->setEnabled (false);
    }
    else
    {
	radioButtonFacesNormal->toggle ();
	tabWidget->setCurrentWidget (faces);
    }
    scaleWidgetColorBar->setVisible (false);
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

void MainWindow::updateStatus ()
{
    QString oldString = labelStatus->text ();
    ostringstream ostr;
    ostr << "Time step " << widgetGl->GetTimeStep ();
    if (! widgetGl->IsDisplayedAllBodies ())
	ostr << ", B: " << (widgetGl->GetDisplayedBodyId () + 1);
    if (widgetGl->GetDisplayedFaceIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", F: " << (widgetGl->GetDisplayedFaceId () + 1);
    if (widgetGl->GetDisplayedEdgeIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", E: " << (widgetGl->GetDisplayedEdgeId () + 1);
    ostr << ends;
    QString newString (ostr.str().c_str ());
    if (oldString != newString)
	labelStatus->setText (newString);
}


void MainWindow::enableBegin ()
{
    if (sliderData->value () > sliderData->minimum ())
        toolButtonBegin->setDisabled (false);
}

void MainWindow::enableEnd ()
{
    if (sliderData->value () < sliderData->maximum ())
        toolButtonEnd->setDisabled (false);
}

void MainWindow::enablePlay ()
{
    if (sliderData->value () < sliderData->maximum ())
        toolButtonPlay->setDisabled (false);
}


void MainWindow::keyPressEvent (QKeyEvent* event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers ();
    switch (event->key ())
    {
    case Qt::Key_Up:
	widgetGl->IncrementDisplayedEdge ();
	updateStatus ();
	break;
    case Qt::Key_Down:
	widgetGl->DecrementDisplayedEdge ();
	updateStatus ();
	break;
    case Qt::Key_PageUp:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->IncrementDisplayedBody ();
	    updateStatus ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->IncrementDisplayedFace ();
	    updateStatus ();
	    break;
	}
        break;
    case Qt::Key_PageDown:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->DecrementDisplayedBody ();
	    updateStatus ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->DecrementDisplayedFace ();
	    updateStatus ();
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
    sliderData->setValue (sliderData->minimum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::ClickedEnd ()
{
    sliderData->setValue (sliderData->maximum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::TimeoutTimer ()
{
    int value = sliderData->value ();
    if (value < sliderData->maximum ())
        sliderData->setValue (value + 1);
    else
        ClickedPlay ();
}

void MainWindow::ValueChangedNone ()
{
    cdbg << "ValueChangedNone: " << endl;
}


void MainWindow::ValueChangedSliderData (int timeStep)
{
    BodyProperty::Enum bodyProperty = 
	BodyProperty::FromSizeT (comboBoxFacesColor->currentIndex ());
    if (widgetHistogram->isVisible ())
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	SetAndDisplayHistogram (
	    checkBoxFacesHistogram->isChecked (),
	    bodyProperty,
	    foamAlongTime.GetHistogram (bodyProperty, timeStep), 
	    foamAlongTime.GetMaxCountPerBinIndividual (bodyProperty),
	    KEEP_SELECTION);
    }
    widgetGl->ValueChangedSliderData (timeStep);
    updateButtons ();
    updateStatus ();
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
    scaleWidgetColorBar->setVisible (
	checked && m_bodyProperty != BodyProperty::NONE);
}


void MainWindow::ToggledCenterPath (bool checked)
{
    if (checked)
    {
	fieldsToControls (comboBoxCenterPathColor, checkBoxCenterPathHistogram);
	ToggledCenterPathHistogram (m_histogram);
	stackedWidgetComposite->setCurrentWidget (pageCenterPath);
    }
    else
	stackedWidgetComposite->setCurrentWidget (pageCompositeEmpty);
    displayHistogramColorBar (checked);
    widgetGl->ToggledCenterPath (checked);
}

void MainWindow::ToggledFacesNormal (bool checked)
{
    if (checked)
    {
	fieldsToControls (comboBoxFacesColor, checkBoxFacesHistogram);
	ToggledFacesHistogram (m_histogram);
	stackedWidgetFaces->setCurrentWidget (pageFacesNormal);
    }
    else
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
    displayHistogramColorBar (checked);
    widgetGl->ToggledFacesNormal (checked);
}


void MainWindow::changedColorBarInterval (const QwtDoubleInterval& interval)
{
    m_colorMapInterval = interval;
    QwtLinearScaleEngine scaleEngine;
    QwtScaleDiv scaleDiv = scaleEngine.divideScale (
	m_colorMapInterval.minValue (), m_colorMapInterval.maxValue (), 0, 0);
    QwtValueList majorTicks;
    majorTicks += 
	(m_colorMapInterval.minValue () + m_colorMapInterval.maxValue()) / 2;
    majorTicks += m_colorMapInterval.minValue ();
    majorTicks += m_colorMapInterval.maxValue ();
    scaleDiv.setTicks(QwtScaleDiv::MajorTick, majorTicks);    
    scaleWidgetColorBar->setScaleDiv (scaleEngine.transformation (), scaleDiv);
    
    //setupBlueRedColorMap (&m_colorMap);
    setupRainbowColorMap (&m_colorMap);
    scaleWidgetColorBar->setColorMap (m_colorMapInterval, m_colorMap);
}

void MainWindow::CurrentIndexChangedFacesColor (int value)
{
    BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (value);
    m_bodyProperty = bodyProperty;
    if (bodyProperty == BodyProperty::NONE)
    {
	widgetGl->CurrentIndexChangedFacesColor (bodyProperty);
	widgetGl->SetColorMap (0, 0);
	checkBoxFacesHistogram->setVisible (false);
	scaleWidgetColorBar->setVisible (false);
	widgetHistogram->setVisible (false);
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	size_t timeStep = widgetGl->GetTimeStep ();
	checkBoxFacesHistogram->setVisible (true);
	scaleWidgetColorBar->setVisible (true);
	widgetGl->CurrentIndexChangedFacesColor (bodyProperty);
	widgetGl->SetColorMap (&m_colorMap, &m_colorMapInterval);
	changedColorBarInterval (
	    foamAlongTime.GetRange (bodyProperty));
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
	widgetGl->SetColorMap (0, 0);
	checkBoxCenterPathHistogram->setVisible (false);
	scaleWidgetColorBar->setVisible (false);
	widgetHistogram->setVisible (false);
    }
    else
    {
	FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
	BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (
	    comboBoxCenterPathColor->currentIndex ());
	checkBoxCenterPathHistogram->setVisible (true);
	scaleWidgetColorBar->setVisible (true);

	widgetGl->SetColorMap (&m_colorMap, &m_colorMapInterval);
	changedColorBarInterval (
	    foamAlongTime.GetRange (bodyProperty));
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
	foamAlongTime.GetMaxCountPerBinIndividual (m_bodyProperty));
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
    HistogramSelection histogramSelection)
{
    if (! checked)
    {
	widgetHistogram->setVisible (false);
	return;
    }

    widgetHistogram->setVisible (true);
    if (histogramSelection == KEEP_SELECTION)
    {
	vector< pair<size_t, size_t> > selectedBins;
	widgetHistogram->GetSelectedBins (&selectedBins);
	widgetHistogram->SetData (intervalData, maxYValue, &selectedBins);
    }
    else
	widgetHistogram->SetData (intervalData, maxYValue);
    widgetHistogram->setAxisTitle (
	QwtPlot::xBottom, 
	QString(BodyProperty::ToString (bodyProperty).c_str ()));
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
/*        
    ostream_iterator<QwtDoubleInterval> out(cdbg, "\n");
    cdbg << "valueIntervals" << endl;
    copy (valueIntervals.begin (), valueIntervals.end (), out);    
*/
    vector<bool> timeStepSelection;
    widgetGl->GetFoamAlongTime ().GetTimeStepSelection (
	bodyProperty, valueIntervals, &timeStepSelection);
/*
    cdbg << "timeStepSelection" << endl
	 << timeStepSelection << endl;
*/
    sliderData->SetRestrictedTo (timeStepSelection);
    if (sliderData->GetState () != RestrictedRangeSlider::FULL_RANGE)
    {
	ostringstream ostr;
	size_t range = 
	    (sliderData->GetState () == RestrictedRangeSlider::EMPTY_RANGE) ? 
	    0 : (sliderData->maximum () - sliderData->minimum () + 1);
	ostr << "Selected Time Steps: " 
	     << range << " of "
	     << (sliderData->GetOriginalMaximum () - 
		 sliderData->GetOriginalMinimum () + 1);
	groupBoxTimeSteps->setTitle (ostr.str ().c_str ());
    }
    else
	groupBoxTimeSteps->setTitle ("Time Steps");
}
