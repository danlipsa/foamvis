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
    m_saveMovie(false), m_currentFrame(0),
    m_centerPathHistogram (false)
{
    // for anti-aliased lines
    QGLFormat fmt;
    fmt.setAlpha(true);
    QGLFormat::setDefaultFormat(fmt);    

    setupUi (this);
    setupSliderData (foamAlongTime);
    setupScaleWidget ();
    widgetGl->SetFoamAlongTime (&foamAlongTime);
    widgetGl->SetColorMap (0, 0);
    widgetHistogram->setVisible (false);
    calculateStats (*foamAlongTime.GetFoam (0), foamAlongTime.GetTimeSteps ());
    updateStatus ();
    m_currentTranslatedBody = widgetGl->GetCurrentFoam ().GetBodies ().begin ();
    configureInterface (foamAlongTime);

    setWindowTitle (
	QString (
	    (string("Foam - ") + foamAlongTime.GetFilePattern ()).c_str ()));
    // 30 ms
    m_timer->setInterval (30);
    QObject::connect(m_timer.get (), SIGNAL(timeout()),
                     this, SLOT(TimeoutTimer ()));
}

void MainWindow::calculateStats (const Foam& foam, size_t timeSteps)
{
    VertexSet vertexSet;
    EdgeSet edgeSet;
    FaceSet faceSet;
    foam.GetVertexSet (&vertexSet);
    foam.GetEdgeSet (&edgeSet);
    foam.GetFaceSet (&faceSet);
    ostringstream ostr;
    ostr << foam.GetBodies ().size () << ","
	 << faceSet.size () << ","
	 << edgeSet.size () << ","
	 << vertexSet.size () << ","
	 << timeSteps;
    m_stats = ostr.str ();
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
    scaleWidgetColorBar->setBorderDist (5, 5);
    scaleWidgetColorBar->setColorBarEnabled (true);
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
    comboBoxColoredBy->setCurrentIndex (CenterPathColor::NONE);
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
    ostr << "(B,F,E,V,T)=" << "(" << m_stats << ")"
	 << " T: " 
	 << (widgetGl->GetTimeStep () + 1);
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
	break;
    }

    case Qt::Key_A:
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
	break;
    }

    case Qt::Key_R:
	if (modifiers == Qt::ShiftModifier)
	    widgetGl->ResetTransformations ();
	else
	    InteractionModeRotate ();
	break;

    case Qt::Key_Z:
	InteractionModeScale ();
	break;
    case Qt::Key_T:
	InteractionModeTranslate ();
	break;
    case Qt::Key_P:
	cdbg << "OpenGL State:" << endl;
	cdbg << G3D::getOpenGLState (false) << endl;
	break;
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

void MainWindow::ValueChangedSliderData (int value)
{
    widgetGl->ValueChangedSliderData (value);
    widgetGl->UpdateDisplayList ();
    updateButtons ();
    updateStatus ();
    if (m_saveMovie)
    {
        ostringstream file;
        file << "movie/frame" << setfill ('0') << setw (4) <<
	    m_currentFrame << ".jpg" << ends;
	cdbg << "Taking snapshot ...";
        QImage snapshot = 
	    QPixmap::grabWindow (widgetDisplay->winId ()).toImage ();
	string f = file.str ();
	cdbg << "Saving " << f << " ..." << endl;
	if (! snapshot.save (f.c_str ()))
	    cdbg << "Error saving " << f << endl;
	m_currentFrame++;
    }
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

void MainWindow::ToggledSaveMovie (bool checked)
{
    m_saveMovie = checked;
    if (checked)
	m_currentFrame = 0;
    ValueChangedSliderData (sliderData->value ());
}

void MainWindow::ToggledCenterPath (bool checked)
{
    widgetGl->ToggledCenterPath (checked);
    if (checked)
    {
	stackedWidgetComposite->setCurrentWidget (pageCenterPath);
	comboBoxColoredBy->setCurrentIndex (CenterPathColor::NONE);
    }
    else
    {
	stackedWidgetComposite->setCurrentWidget (pageCompositeEmpty);
	scaleWidgetColorBar->setVisible (false);
	widgetHistogram->setVisible (false);	
    }
}


void MainWindow::changeScaleWidgetInterval (CenterPathColor::Enum colorBy)
{
    const FoamAlongTime& foamAlongTime = widgetGl->GetFoamAlongTime ();
    m_colorMapInterval = QwtDoubleInterval(
	foamAlongTime.GetMin(colorBy), 
	foamAlongTime.GetMax(colorBy));
    cdbg << "changeScaleWidgetInterval: " << colorBy << " ("
	 << m_colorMapInterval.minValue () << ", " 
	 << m_colorMapInterval.maxValue () << ")" << endl;

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


void MainWindow::ValueChangedColoredBy (int value)
{
    CenterPathColor::Enum colorBy = 
	static_cast<CenterPathColor::Enum>(value);
    if (colorBy == CenterPathColor::NONE)
    {
	widgetGl->SetColorMap (0, 0);
	scaleWidgetColorBar->setVisible (false);
	checkBoxCenterPathHistogram->setVisible (false);
	widgetHistogram->setVisible (false);
    }
    else
    {
	checkBoxCenterPathHistogram->setVisible (true);
	widgetGl->SetColorMap (&m_colorMap, &m_colorMapInterval);
	changeScaleWidgetInterval (colorBy);
	scaleWidgetColorBar->setVisible (true);
	widgetGl->ValueChangedCenterPathColor (colorBy);
	SetAndDisplayHistogram ();
    }
    widgetGl->UpdateDisplayList ();
}


void MainWindow::ToggledCenterPathHistogram (bool checked)
{
    m_centerPathHistogram = checked;
    SetAndDisplayHistogram ();
}

void MainWindow::SetAndDisplayHistogram ()
{
    if (m_centerPathHistogram)
    {
	size_t value = comboBoxColoredBy->currentIndex ();
	CenterPathColor::Enum colorBy = 
	    static_cast<CenterPathColor::Enum>(value);
	widgetHistogram->setVisible (true);
	widgetHistogram->SetData (
	    widgetGl->GetFoamAlongTime ().GetBodiesAlongTime ().
	    GetHistogram (colorBy));
	widgetHistogram->setTitle (
	    QString(CenterPathColor::ToString (colorBy).c_str()));
	widgetHistogram->replot ();
    }
    else
	widgetHistogram->setVisible (false);
}
