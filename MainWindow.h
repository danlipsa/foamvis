/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains the all the user interface
 */
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "ui_MainWindow.h"
#include "Enums.h"
#include "Foam.h"

class ProcessBodyTorus;
class QTimer;
class Settings;
class SimulationGroup;
class AverageCache;
class WidgetGl;

/**
 * Class that stores the OpenGL, Vtk, Histogram and  UI widgets.
 */
class MainWindow : public QMainWindow, private Ui::MainWindow
{
public:
    enum PlayType
    {
	PLAY_FORWARD,
	PLAY_REVERSE
    };
    
    typedef pair<QwtIntervalData, size_t> HistogramInfo;

public:
    /**
     * Constructor
     * @param simulationGroup data to be displayed
     */
    MainWindow(SimulationGroup& simulationGroup);
    /**
     * Called when a key is pressed
     * @param event object describing the key
     */
    void keyPressEvent (QKeyEvent* event);

Q_SIGNALS:
    void BodyOrFaceScalarChanged (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel, size_t property);
    void ColorBarModelChanged (ViewNumber::Enum viewNumber, 
			       boost::shared_ptr<ColorBarModel> colorBarModel);
    void OverlayBarModelChanged (ViewNumber::Enum viewNumber, 
				 boost::shared_ptr<ColorBarModel> colorBarModel);


public Q_SLOTS:
    void SelectionChangedSettings (ViewNumber::Enum viewNumber);
    void ToggledViewFocusShown (bool checked);
    void ToggledTitleShown (bool checked);
    void ToggledBubblePathsLineUsed (bool checked);
    void ValueChangedContextAlpha (int sliderValue);
    void ToggledVelocityShown (bool checked);
    void ToggledHistogramGridShown (bool checked);
    void ToggledHistogramShown (bool checked);
    void ToggledHistogramColorMapped (bool checked);
    void ToggledHistogramAllTimesteps (bool checked);

    void ButtonClickedViewType (int viewType);
    void ButtonClickedVelocityVis (int velocityVis);

    void ToggledReflectedHalfView (bool checked);
    void ToggledForceDifference (bool checked);

    /**
     * Called when the Begin Slider button is pressed. Shows the first
     * data in the vector.
     */
    void ClickedBegin ();
    /**
     * Called when  the End Slider  button is pressed. Shows  the last
     * data in the vector.
     */
    void ClickedEnd ();
    /**
     * Called when  the user pushed  the play button. Starts  or stops
     * showing the time steps of the Surface Evolver data.
     */
    void ClickedPlay ();
    void ClickedPlayReverse ();
    void CurrentIndexChangedSimulation (int index);
    void CurrentIndexChangedInteractionMode (int index);
    void CurrentIndexChangedFaceColor (int value);
    void CurrentIndexChangedStatisticsType (int value);
    /**
     * Copy status from WidgetGl to the UI controls
     */
    void CurrentIndexChangedSelectedLight (int i);
    void CurrentIndexChangedViewCount (int index);
    void CurrentIndexChangedViewLayout (int index);
    void CurrentIndexChangedWindowLayout (int index);

    void ShowEditColorMap ();
    void ShowEditOverlayMap ();
    void RotateShown ();
    void ScaleShown ();
    void TranslateShown ();    
    void SelectShown ();
    void DeselectShown ();

    void SelectionChangedHistogram (int viewNumber);
    /**
     * Invoqued by the timer to show the next data in the vector
     */
    void TimeoutTimer ();
    
    /**
     * Invoqued by the UI when  the slider changes to display the data
     * at the current position of the slider.
     */
    void ValueChangedFontSize (int value);
    void ValueChangedHistogramHeight (int value);
    void ValueChangedSliderTimeSteps (int value);
    void ValueChangedAverageTimeWindow (int timeSteps);
    void ValueChangedT1sKernelSigma (double value);
    void ViewToUI (ViewNumber::Enum prevViewNumber);
    void SetHistogramColorBarModel (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel);

private:
    void initOverlayBarModel ();
    static void clearStretch (QWidget* widget);
    static void updateStretch (QWidget* widget, 
			       ViewLayout::Enum layout,
			       size_t value);
    void updateSliderTimeSteps (
        ViewNumber::Enum viewNumber,
        const vector<QwtDoubleInterval>& valueIntervals);
    void updateStretch ();
    void addVtkView (ViewNumber::Enum viewNumber);
    void update3DAverage ();
    void currentIndexChangedFaceColor (ViewNumber::Enum viewNumber);
    void deformationViewToUI ();
    void velocityViewToUI ();
    void forceViewToUI ();
    void t1sKDEViewToUI ();
    void bubblePathsViewToUI ();
    HistogramInfo createHistogramInfo (
	pair<float, float> minMax, size_t count) const;    
    void setStackedWidget (ViewType::Enum viewType);
    void updateLightControls (
	const ViewSettings& vs, LightNumber::Enum lightNumber);
    void connectSignals ();
    void connectColorBarHistogram (bool connected);
    /**
     * Enables/Disables the Begin button
     * @param enable true to enable and false to disable the button
     */
    void enableBegin (bool enable);
    /**
     * Enables/Disables the End button
     * @param enable true to enable and false to disable the button
     */
    void enableEnd (bool enable);
    /**
     * Enables/Disables the Play button
     * @param enable true to enable and false to disable the button
     */
    void enablePlayForward ();
    void enablePlayReverse ();
    /**
     * Enables/Disables all buttons based on the position of the slider.
     */
    void updateButtons ();
    /**
     * Updates the status bar based on the postion of the slider.
     */
    void setupSliderData (const Simulation& simulation);
    void setupButtonGroups ();
    void configureInterfaceDataDependent (
	const SimulationGroup& simulationGroup);
    void configureInterface ();
    void setupColorBarModelBodyScalar (
	size_t simulationIndex, 
	ViewNumber::Enum viewNumber, BodyScalar::Enum property);
    void setupColorBarModelDomainHistogram (
	size_t simulationIndex, ViewNumber::Enum viewNumber);
    void setupColorBarModelVelocityVector (
	size_t simulationIndex, ViewNumber::Enum viewNumber);
    void setupColorBarModelT1sKDE (
	size_t simulationIndex, ViewNumber::Enum viewNumber);

    void setupColorBarModel (
	boost::shared_ptr<ColorBarModel>& colorBarModel, 
	BodyScalar::Enum property, size_t simulationIndex);
    void setupColorBarModels (size_t simulationIndex, 
			      ViewNumber::Enum viewNumber);
    void setupColorBarModels ();
    void setupViews ();

    void processBodyTorusStep ();
    void translatedBodyInit ();
    void initComboBoxSimulation (SimulationGroup& simulationGroup);
    void translatedBodyStep ();
    void createActions ();
    HistogramInfo getHistogramInfo (
	ColorBarType::Enum colorBarType, size_t bodyOrFaceScalar) const;
    boost::shared_ptr<ColorBarModel> getColorBarModel () const;
    boost::shared_ptr<ColorBarModel> getColorBarModel (
	ViewNumber::Enum viewNumber) const;
    boost::shared_ptr<ColorBarModel> getColorBarModel (
	size_t simulationIndex,
	ViewNumber::Enum viewNumber,
	ViewType::Enum viewType, size_t property, 
	StatisticsType::Enum statisticsType) const;
    void clickedPlay (PlayType playType);

private:
    /**
     * Character used on the play button, when we can start the play
     */
    static const char* PLAY_FORWARD_TEXT;
    static const char* PLAY_REVERSE_TEXT;
    /**
     * Character used on the play button when we can pause the play
     */
    static const char* PAUSE_TEXT;


private:
    Q_OBJECT
    /**
     * Timer used  in displaying the  data files. Otherwise  you would
     * display the files too fast.
     */    
    boost::scoped_ptr<QTimer> m_timer;
    boost::shared_ptr<Settings> m_settings;
    
    ////////////
    // Debug PBC
    ProcessBodyTorus* m_processBodyTorus;
    bool m_debugTranslatedBody;
    Foam::Bodies::iterator m_currentTranslatedBody;
    size_t m_currentBody;

    //////////
    // Actions
    boost::shared_ptr<QAction> m_actionRotateShown;
    boost::shared_ptr<QAction> m_actionTranslateShown;
    boost::shared_ptr<QAction> m_actionScaleShown;
    boost::shared_ptr<QAction> m_actionSelectShown;
    boost::shared_ptr<QAction> m_actionDeselectShown;
    
    // index order: simulation index, view number, body property
    vector <
	boost::array<
	    boost::array<boost::shared_ptr<ColorBarModel>, 
			 BodyScalar::COUNT>,
	    ViewNumber::COUNT> > m_colorBarModelBodyScalar;
    // index order: simulation index, view number
    vector <
	boost::array<boost::shared_ptr<ColorBarModel>, 
		     ViewNumber::COUNT> > m_overlayBarModelVelocityVector;
    // index order: simulation index, view number
    vector <
	boost::array<boost::shared_ptr<ColorBarModel>,
		     ViewNumber::COUNT> > m_colorBarModelDomainHistogram;
    // index order: simulation index, view number
    vector <boost::array<boost::shared_ptr<ColorBarModel>,
			 ViewNumber::COUNT> > m_colorBarModelT1sKDE;
    boost::shared_ptr<EditColorMap> m_editColorMap;
    /**
     * True if the program displays data in a loop, false
     * otherwise
     */
    bool m_playForward;
    bool m_playReverse;
    const SimulationGroup& m_simulationGroup;
    boost::array<boost::shared_ptr<AverageCache>, 
                 ViewNumber::COUNT> m_averageCache;
};


#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

