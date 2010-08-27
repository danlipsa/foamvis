/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains the all the user interface
 */
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "ui_Window.h"
#include "Enums.h"
#include "Foam.h"

class QTimer;
class GLWidget;
class FoamAlongTime;
class ProcessBodyTorus;

/**
 * Class that contains the OpenGL widget and all other UI.
 */
class MainWindow : public QWidget, private Ui::Window
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
	REPLACE_MAX_VALUE,
    };

    enum HistogramDisplay
    {
	NONE,
	UNICOLOR,
	COLOR_CODED
    };


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
     * @param checked true for showing the histogram, false otherwise
     * @param intervalData data for the histogram
     */
    void SetAndDisplayHistogram (
	bool checked,
	BodyProperty::Enum bodyProperty,
	const QwtIntervalData& intervalData,
	double maxValue,
	HistogramSelection histogramSelection = DISCARD_SELECTION,
	MaxValueOperation maxValueOperation = REPLACE_MAX_VALUE);


public Q_SLOTS:
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

    void CurrentIndexChangedCenterPathColor (int value);
    void CurrentIndexChangedFacesColor (int value);


    void InteractionModeRotate ();
    void InteractionModeSelectBrush ();
    void InteractionModeSelectEraser ();
    void InteractionModeScale ();
    void InteractionModeTranslate ();
    
    void SelectionChangedHistogram ();

    /**
     * Invoqued by the timer to show the next data in the vector
     */
    void TimeoutTimer ();

    void ToggledCenterPath (bool checked);

    void ToggledHistogramNone (bool checked);
    void ToggledCenterPathHistogramUnicolor (bool checked);
    void ToggledCenterPathHistogramColorCoded (bool checked);

    void ToggledFacesNormal (bool checked);
    void ToggledFacesHistogramUnicolor (bool checked);
    void ToggledFacesHistogramColorCoded (bool checked);

    void ToggledFacesTorus (bool checked);

    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ToggledEdgesNormal (bool checked);
    void ToggledEdgesTorus (bool checked);
    /**
     * Invoqued by the UI when  the slider changes to display the data
     * at the current position of the slider.
     */
    void ValueChangedSliderTimeSteps (int value);
private:
    /**
     * Enables/Disables the Begin button
     * @param enable true to enable and false to disable the button
     */
    void enableBegin ();
    /**
     * Enables/Disables the End button
     * @param enable true to enable and false to disable the button
     */
    void enableEnd ();
    /**
     * Enables/Disables the Play button
     * @param enable true to enable and false to disable the button
     */
    void enablePlay ();
    /**
     * Enables/Disables all buttons based on the position of the slider.
     */
    void updateButtons ();
    /**
     * Updates the status bar based on the postion of the slider.
     */
    void setupSliderData (const FoamAlongTime& foamAlongTime);
    void configureInterface (const FoamAlongTime& foamAlongTime);
    void changedColorBarInterval (BodyProperty::Enum bodyProperty);

    void setupColorBar ();
    void setupHistogram ();
    void processBodyTorusStep ();
    void translateBodyStep ();
    void createActions ();
    void fieldsToControls (QComboBox* comboBox, QCheckBox* checkBox);
    void displayHistogramColorBar (bool checked);

private:
    Q_OBJECT
    /**
     * True if the programs displays the data files iteratively, false
     * otherwise
     */
    bool m_play;
    /**
     * Character used on the play button, when we can start the play
     */
    const char* PLAY_TEXT;
    /**
     * Character used on the play button when we can pause the play
     */
    const char* PAUSE_TEXT;
    /**
     * Timer used  in displaying the  data files. Otherwise  you would
     * display the files too fast.
     */
    boost::scoped_ptr<QTimer> m_timer;
    ProcessBodyTorus* m_processBodyTorus;
    Foam::Bodies::iterator m_currentTranslatedBody;
    size_t m_currentBody;
    boost::shared_ptr<ColorBarModel> m_colorBarModel;

    boost::shared_ptr<QAction> m_actionRotate;
    boost::shared_ptr<QAction> m_actionTranslate;
    boost::shared_ptr<QAction> m_actionScale;
    boost::shared_ptr<QAction> m_actionSelectBrush;
    boost::shared_ptr<QAction> m_actionSelectEraser;
    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionInfo;

    BodyProperty::Enum m_bodyProperty;
    HistogramDisplay m_histogramDisplay;
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

