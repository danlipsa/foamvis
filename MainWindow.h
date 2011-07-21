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
class FoamAlongTime;
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
     * @param foamAlongTime data to be displayed read from Surface Evolver files
     */
    MainWindow(FoamAlongTime& foamAlongTime);
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
    void BodyPropertyChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	BodyProperty::Enum property);
    void ColorBarModelChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
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
    void CurrentIndexChangedFacesColor (int value);
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
    void RotateLightShown ();
    void TranslateLightShown ();

    void SelectionChangedHistogram ();
    /**
     * Invoqued by the timer to show the next data in the vector
     */
    void TimeoutTimer ();
    void ToggledCenterPath (bool checked);
    
    void ButtonClickedHistogram (int histogramType);
    void ToggledFacesNormal (bool checked);
    void ToggledFacesStatistics (bool checked);
    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ToggledEdgesNormal (bool checked);
    /**
     * Invoqued by the UI when  the slider changes to display the data
     * at the current position of the slider.
     */
    void ValueChangedSliderTimeSteps (int value);
    void ValueChangedFontSize (int value);
    void ValueChangedHistogramHeight (int value);
    void ViewToUI ();
    void SetHistogramColorBarModel (
	boost::shared_ptr<ColorBarModel> colorBarModel);

private:
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
    void setupSliderData (const FoamAlongTime& foamAlongTime);
    void setupButtonGroups ();
    void configureInterface (const FoamAlongTime& foamAlongTime);
    void setupColorBarModel (ViewNumber::Enum viewNumber,
			     BodyProperty::Enum property);
    void setupColorBarModels (ViewNumber::Enum viewNumber);

    void setupHistogram ();
    void processBodyTorusStep ();
    void translateBodyStep ();
    void createActions ();
    void displayHistogramColorBar (bool checked);
    HistogramInfo getCurrentHistogramInfo () const;
    boost::shared_ptr<ColorBarModel> getCurrentColorBarModel () const;
    void clickedPlay (PlayType playType);
    void setDefaults ();

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
    Foam::Bodies::iterator m_currentTranslatedBody;
    size_t m_currentBody;

    boost::shared_ptr<QAction> m_actionRotateShown;
    boost::shared_ptr<QAction> m_actionTranslateShown;
    boost::shared_ptr<QAction> m_actionScaleShown;
    boost::shared_ptr<QAction> m_actionRotateLightShown;
    boost::shared_ptr<QAction> m_actionTranslateLightShown;
    boost::shared_ptr<QAction> m_actionSelectShown;
    boost::shared_ptr<QAction> m_actionDeselectShown;

    HistogramType::Enum m_histogramType;
    ViewNumber::Enum m_histogramViewNumber;
    boost::array<
	boost::array<boost::shared_ptr<ColorBarModel>, 
		     BodyProperty::PROPERTY_END>,
	ViewNumber::COUNT> m_colorBarModelBodyProperty;
    boost::array<
	boost::shared_ptr<ColorBarModel>,
	ViewNumber::COUNT> m_colorBarModelDomainHistogram;
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

