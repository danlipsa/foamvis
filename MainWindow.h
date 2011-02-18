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
	REPLACE_MAX_VALUE,
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
	HistogramType::Enum histogramType,
	BodyProperty::Enum property,
	const QwtIntervalData& intervalData,
	double maxValue,
	HistogramSelection histogramSelection = DISCARD_SELECTION,
	MaxValueOperation maxValueOperation = REPLACE_MAX_VALUE);
    boost::shared_ptr<ColorBarModel> GetColorBarModel (
	BodyProperty::Enum property)
    {
	return m_colorBarModel[property];
    }

Q_SIGNALS:
    void BodyPropertyChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	BodyProperty::Enum property, ViewType::Enum viewType);
    void ColorBarModelChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
    void ToggledTimeStepsShown (bool checked);
    void ToggledColorBarShown (bool checked);
    void ToggledHistogramGridShown (bool checked);
    void ToggledStatusBarShown (bool checked);

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
    /**
     * Copy status from GLWidget to the UI controls
     */
    void CurrentIndexChangedSelectedLight (int i);

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
    
    void ButtonClickedOneTimestepHistogram (int id);
    void ButtonClickedAllTimestepsHistogram (int id);
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
private:
    bool isHistogramHidden (HistogramType::Enum histogramType);
    void connectColorBarModelChanged ();
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
    void setupButtonGroups ();
    void configureInterface (const FoamAlongTime& foamAlongTime);
    void setupColorBarModel (BodyProperty::Enum property);
    void setupColorBarModels ();

    void setupColorBar ();
    void setupHistogram ();
    void setVisibleFacesHistogramSelection (bool visible);
    void setVisibleCenterPathHistogramSelection (bool visible);
    void processBodyTorusStep ();
    void translateBodyStep ();
    void createActions ();
    void fieldsToControls (QComboBox* comboBox, QButtonGroup* buttonGroup);
    void displayHistogramColorBar (bool checked);

private:
    Q_OBJECT
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

    boost::shared_ptr<QAction> m_actionRotateShown;
    boost::shared_ptr<QAction> m_actionTranslateShown;
    boost::shared_ptr<QAction> m_actionScaleShown;
    boost::shared_ptr<QAction> m_actionRotateLightShown;
    boost::shared_ptr<QAction> m_actionTranslateLightShown;
    boost::shared_ptr<QAction> m_actionSelectShown;
    boost::shared_ptr<QAction> m_actionDeselectShown;

    BodyProperty::Enum m_property;
    HistogramType::Enum m_histogramType;
    vector< boost::shared_ptr<ColorBarModel> > m_colorBarModel;
    boost::shared_ptr<EditColorMap> m_editColorMap;
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

