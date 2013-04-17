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
#include "Base.h"
#include "Base.h"

class ProcessBodyTorus;
class QTimer;
class Settings;
class SimulationGroup;
class AverageCache;

/**
 * Class that stores the OpenGL, Vtk, Histogram and  UI widgets.
 */
class MainWindow : public QMainWindow, public Base, private Ui::MainWindow
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
    MainWindow(boost::shared_ptr<const SimulationGroup> simulationGroup);
    /**
     * Called when a key is pressed
     * @param event object describing the key
     */
    void keyPressEvent (QKeyEvent* event);
    void ShowMessageBox (const char* message);

Q_SIGNALS:
    void BodyOrFaceScalarChanged (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel, size_t property);
    void ColorMapScalarChanged (ViewNumber::Enum viewNumber, 
                                boost::shared_ptr<ColorBarModel> colorBarModel);
    void ColorMapVelocityChanged (
        ViewNumber::Enum viewNumber, 
        boost::shared_ptr<ColorBarModel> colorBarModel);


public Q_SLOTS:
    void SelectionChangedFromSettings (ViewNumber::Enum viewNumber);
    void ToggledT1KDEKernelBoxShown (bool checked);
    void ToggledBarLarge (bool large);
    void ToggledAxesShown (bool checked);
    void ToggledBoundingBoxSimulation (bool checked);
    void ToggledScalarShown (bool shown);
    void ToggledScalarContext (bool context);
    void ToggledViewFocusShown (bool checked);
    void ToggledTitleShown (bool checked);
    void ToggledBubblePathsLineUsed (bool checked);
    void ValueChangedContextAlpha (int sliderValue);
    void ValueChangedObjectAlpha (int sliderValue);
    void ValueChangedForceSize (int index);
    void ValueChangedForceLineWidth (int index);
    void ValueChangedGlyphSeedsCount (int index);
    void ToggledVelocityShown (bool checked);
    void ToggledHistogramGridShown (bool checked);
    void ToggledHistogramShown (bool checked);
    void ToggledHistogramColor (bool checked);
    void ToggledHistogramAllTimesteps (bool checked);
    void ToggledVelocitySameSize (bool checked);

    void CellClickedLinkedTimeEvents (int row, int column);
    void ButtonClickedInteractionObject (int id);
    void ButtonClickedViewType (int viewType);
    void ButtonClickedForce (int type);
    void ButtonClickedTorque (int type);
    void ButtonClickedVelocityVis (int velocityVis);
    void ButtonClickedTimeLinkage (int id);

    void ToggledTwoHalvesView (bool checked);

    /**
     * Called when the Begin Slider button is pressed. Shows the first
     * data in the vector.
     */
    void ButtonClickedBegin ();
    /**
     * Called when  the End Slider  button is pressed. Shows  the last
     * data in the vector.
     */
    void ButtonClickedEnd ();
    /**
     * Called when  the user pushed  the play button. Starts  or stops
     * showing the time steps of the Surface Evolver data.
     */
    void ButtonClickedPlay ();
    void ButtonClickedPlayReverse ();
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
    
    void EditColorMapScalar ();
    void ClampClearColorMapScalar ();
    void CopyColorMapScalar (int viewNumber);
    void CopyColorMapVelocity (int other);
    void EditColorMapVelocity ();
    void ClampClearColorMapVelocity ();
    void CopyColorMapVelocityFromScalar ();

    void RotateShown ();
    void ScaleShown ();
    void TranslateShown ();    
    void SelectShown ();
    void DeselectShown ();
    void ResetTransformAll ();

    void SelectionChangedFromHistogram (int viewNumber);
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
    void ValueChangedT1KDEKernelSigma (double value);
    void ValueChangedT1KDEIsosurfaceValue (double value);
    void ViewToUI (ViewNumber::Enum prevViewNumber);
    void SetHistogramColorBarModel (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel);    

private:
    void initColorMapVelocity ();
    static void clearStretch (QWidget* widget);
    static void updateStretch (QWidget* widget, 
			       ViewLayout::Enum layout,
			       size_t value);
    void updateSliderTimeSteps (
        ViewNumber::Enum viewNumber,
        const vector<QwtDoubleInterval>& valueIntervals);
    void updateStretch ();
    void addVtkView (ViewNumber::Enum viewNumber);
    void updateAllViews3DAverage ();
    void currentIndexChangedFaceColor (ViewNumber::Enum viewNumber);
    void settingsViewToUI (ViewNumber::Enum viewNumber);
    void timeViewToUI (ViewNumber::Enum viewNumber);
    void linkedTimeEventsViewToUI (ViewNumber::Enum viewNumber);
    void deformationViewToUI ();
    void velocityViewToUI ();
    void forceViewToUI ();
    void t1KDEViewToUI (ViewNumber::Enum viewNumber);
    void bubblePathsViewToUI ();
    void lightViewToUI (
	const ViewSettings& vs, LightNumber::Enum lightNumber);
    void histogramViewToUI ();
    HistogramInfo createHistogramInfo (
	pair<float, float> minMax, size_t count) const;    
    void setStackedWidgetVisualization (ViewType::Enum viewType);
    void setStackedWidgetVelocity (VectorVis::Enum vis);
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
    void setupColorBarModelScalar (
	size_t simulationIndex, 
	ViewNumber::Enum viewNumber, BodyScalar::Enum property);
    void setupColorBarModelDomainHistogram (
	size_t simulationIndex, ViewNumber::Enum viewNumber);
    void setupColorBarModelVelocity (
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
    void initComboBoxSimulation (const SimulationGroup& simulationGroup);
    void translatedBodyStep ();
    void createActions ();
    HistogramInfo getHistogramInfo (
	ColorMapScalarType::Enum colorBarType, size_t bodyOrFaceScalar) const;
    QwtDoubleInterval getScalarInterval (ViewNumber::Enum viewNumber);
    boost::shared_ptr<ColorBarModel> getColorMapScalar () const;
    boost::shared_ptr<ColorBarModel> getColorMapScalar (
	ViewNumber::Enum viewNumber) const;
    boost::shared_ptr<ColorBarModel> getColorMapScalar (
        ViewNumber::Enum viewNumber, size_t bodyAttribute) const;
    boost::shared_ptr<ColorBarModel> getColorMapScalar (
	size_t simulationIndex,
	ViewNumber::Enum viewNumber,
	ViewType::Enum viewType, size_t property, 
	StatisticsType::Enum statisticsType) const;
    boost::shared_ptr<ColorBarModel> getColorMapVelocity (    
        ViewNumber::Enum viewNumber) const;
    void clickedPlay (PlayType playType);
    void toggledT1KDEKernelBoxShown (ViewNumber::Enum viewNumber);
    void valueChangedT1KDEKernelSigma (ViewNumber::Enum viewNumber);

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
    // Shortcuts for actions shared between GL and VTK
    boost::shared_ptr<QAction> m_actionResetTransformAll;
    
    // index order: simulation index, view number, body property
    vector <
	boost::array<
	    boost::array<boost::shared_ptr<ColorBarModel>, 
			 BodyScalar::COUNT>,
                             ViewNumber::COUNT> > m_colorMapScalar;
    // index order: simulation index, view number
    vector <
	boost::array<boost::shared_ptr<ColorBarModel>, 
		     ViewNumber::COUNT> > m_colorMapVelocity;
    // index order: simulation index, view number
    vector <
	boost::array<boost::shared_ptr<ColorBarModel>,
		     ViewNumber::COUNT> > m_colorMapDomainHistogram;
    // index order: simulation index, view number
    vector <boost::array<boost::shared_ptr<ColorBarModel>,
			 ViewNumber::COUNT> > m_colorMapT1sKDE;
    boost::shared_ptr<EditColorMap> m_editColorMap;
    /**
     * True if the program displays data in a loop, false
     * otherwise
     */
    bool m_playForward;
    bool m_playReverse;
    boost::array<boost::shared_ptr<AverageCache>, 
                 ViewNumber::COUNT> m_averageCache;
};


#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

