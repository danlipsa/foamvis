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

class QTimer;
class GLWidget;
class SimulationGroup;
class ProcessBodyTorus;

/**
 * Class that contains the OpenGL widget and all other UI.
 */
class MainWindow : public QMainWindow, private Ui::MainWindow
{
public:
    enum HistogramSelection
    {
	KEEP_SELECTION,
	DISCARD_SELECTION
    };

    enum MaxValueOperation
    {
	KEEP_MAX_VALUE,
	REPLACE_MAX_VALUE
    };
    enum PlayType
    {
	PLAY_FORWARD,
	PLAY_REVERSE
    };
    
    typedef pair<QwtIntervalData, size_t> HistogramInfo;

public:
    /**
     * Constructor
     * @param simulation data to be displayed read from Surface Evolver files
     */
    MainWindow(SimulationGroup& simulationGroup);
    /**
     * Called when a key is pressed
     * @param event object describing the key
     */
    void keyPressEvent (QKeyEvent* event);

    /**
     * Shows a histogram of the current display
     */
    void SetAndDisplayHistogram (
	HistogramSelection histogramSelection,
	MaxValueOperation maxValueOperation);


Q_SIGNALS:
    void BodyOrFacePropertyChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel, size_t property);
    void ColorBarModelChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
    void ButtonClickedHistogram (int histogramType);
    void ButtonClickedViewType (int viewType);

    void ToggledHistogramGridShown (bool checked);

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
    void CurrentIndexChangedViewCount (int index);
    /**
     * Copy status from GLWidget to the UI controls
     */
    void CurrentIndexChangedSelectedLight (int i);
    void CurrentIndexChangedWindowSize (int i);

    void ShowEditColorMap ();
    void RotateShown ();
    void ScaleShown ();
    void TranslateShown ();    
    void SelectShown ();
    void DeselectShown ();

    void SelectionChangedHistogram ();
    /**
     * Invoqued by the timer to show the next data in the vector
     */
    void TimeoutTimer ();
    
    /**
     * Invoqued by the UI when  the slider changes to display the data
     * at the current position of the slider.
     */
    void ValueChangedSliderTimeSteps (int value);
    void ValueChangedFontSize (int value);
    void ValueChangedHistogramHeight (int value);
    void ValueChangedT1sKernelSigma (int index);
    void ViewToUI ();
    void SetHistogramColorBarModel (
	boost::shared_ptr<ColorBarModel> colorBarModel);

private:
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
    void setupColorBarModel (size_t simulationIndex, 
			     ViewNumber::Enum viewNumber,
			     BodyProperty::Enum property);
    void setupColorBarModels (size_t simulationIndex, 
			      ViewNumber::Enum viewNumber);
    void setupColorBarModels ();
    void setupViews ();

    void setupHistogram ();
    void processBodyTorusStep ();
    void translatedBodyInit ();
    void initComboBoxSimulation (SimulationGroup& simulationGroup);
    void translatedBodyStep ();
    void createActions ();
    void displayHistogramColorBar (bool checked);
    HistogramInfo getCurrentHistogramInfo () const;
    boost::shared_ptr<ColorBarModel> getColorBarModel () const;
    boost::shared_ptr<ColorBarModel> getColorBarModel (
	size_t simulationIndex,
	ViewNumber::Enum viewNumber,
	ViewType::Enum viewType, size_t property, 
	StatisticsType::Enum statisticsType) const;
    void emitColorBarModelChanged (
	size_t simulationIndex, ViewNumber::Enum viewNumber,
	ViewType::Enum viewType, 
	size_t property, StatisticsType::Enum statisticsType);
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
    ProcessBodyTorus* m_processBodyTorus;
    
    bool m_debugTranslatedBody;
    Foam::Bodies::iterator m_currentTranslatedBody;

    size_t m_currentBody;

    boost::shared_ptr<QAction> m_actionRotateShown;
    boost::shared_ptr<QAction> m_actionTranslateShown;
    boost::shared_ptr<QAction> m_actionScaleShown;
    boost::shared_ptr<QAction> m_actionSelectShown;
    boost::shared_ptr<QAction> m_actionDeselectShown;

    HistogramType::Enum m_histogramType;
    ViewNumber::Enum m_histogramViewNumber;
    /*
     * Index order: simulation index, view number, body property
     */
    vector <
	boost::array<
	    boost::array<boost::shared_ptr<ColorBarModel>, 
			 BodyProperty::COUNT>,
	    ViewNumber::COUNT> > m_colorBarModelBodyProperty;
    vector <
	boost::array<boost::shared_ptr<ColorBarModel>,
		     ViewNumber::COUNT> > m_colorBarModelDomainHistogram;
    vector <boost::array<boost::shared_ptr<ColorBarModel>,
		     ViewNumber::COUNT> > m_colorBarModelT1sPDE;

    boost::shared_ptr<EditColorMap> m_editColorMap;
    /**
     * True if the program displays data in a loop, false
     * otherwise
     */
    bool m_playForward;
    bool m_playReverse;
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

