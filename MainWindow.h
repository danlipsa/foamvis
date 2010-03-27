/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains the all the user interface
 */
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "ui_Window.h"

class QTimer;
class GLWidget;
class DataFiles;
class ProcessBodyTorus;

/**
 * Class that contains the OpenGL widget and all other UI.
 */
class MainWindow : public QWidget, private Ui::Window
{
public:
    /**
     * Constructor
     * @param dataFiles data to be displayed read from Surface Evolver files
     */
    MainWindow(DataFiles& dataFiles);
    /**
     * Called when a key is pressed
     * @param event object describing the key
     */
    void keyPressEvent (QKeyEvent* event);

public Q_SLOTS:
    /**
     * Called when  the user pushed  the play button. Starts  or stops
     * showing the time steps of the Surface Evolver data.
     */
    void TogglePlay ();
    /**
     * Called when the Begin Slider button is pressed. Shows the first
     * data in the vector.
     */
    void BeginSlider ();
    /**
     * Called when  the End Slider  button is pressed. Shows  the last
     * data in the vector.
     */
    void EndSlider ();
    /**
     * Invoqued by the timer to show the next data in the vector
     */
    void IncrementSlider ();
    /**
     * Invoqued by the UI when  the slider changes to display the data
     * at the current position of the slider.
     */
    void DataSliderValueChanged (int value);
        /**
     * Shows vertices
     * @param checked true for showing vertices false otherwise
     */
    void ViewVertices (bool checked);
    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ViewEdges (bool checked);
    /**
     * Shows faces
     * @param checked true for showing faces false otherwise
     */
    void ViewFaces (bool checked);
    /**
     * Shows bodies
     * @param checked true for showing bodies false otherwise
     */
    void ViewBodies (bool checked);
    /**
     * Shows center paths
     * param checked true for showing center paths false otherwise
     */
    void ViewCenterPaths (bool checked);
    void InteractionModeRotate ();
    void InteractionModeScale ();
    void InteractionModeTranslate ();

private:
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
    void enablePlay (bool enable);
    /**
     * Enables/Disables all buttons based on the position of the slider.
     */
    void updateButtons ();
    /**
     * Updates the status bar based on the postion of the slider.
     */
    void updateStatus ();

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
    size_t m_currentBody;
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

