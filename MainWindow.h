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
    /**
     * Constructor
     * @param dataAlongTime data to be displayed read from Surface Evolver files
     */
    MainWindow(FoamAlongTime& dataAlongTime);
    /**
     * Called when a key is pressed
     * @param event object describing the key
     */
    void keyPressEvent (QKeyEvent* event);

    /**
     * Shows center paths
     * param checked true for showing center paths false otherwise
     */
    void InteractionModeRotate ();
    void InteractionModeScale ();
    void InteractionModeTranslate ();


public Q_SLOTS:
    /**
     * Called when  the user pushed  the play button. Starts  or stops
     * showing the time steps of the Surface Evolver data.
     */
    void ClickedPlay ();
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
     * Invoqued by the timer to show the next data in the vector
     */
    void TimeoutTimer ();
    /**
     * Invoqued by the UI when  the slider changes to display the data
     * at the current position of the slider.
     */
    void ValueChangedSliderData (int value);
        /**
     * Shows vertices
     * @param checked true for showing vertices false otherwise
     */
    void ToggledVerticesPhysical (bool checked);


    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ToggledEdgesNormal (bool checked);
    void ToggledEdgesPhysical (bool checked);
    void ToggledEdgesTorus (bool checked);
    void ToggledFacesTorus (bool checked);

    /**
     * Save JPG images of the widgetDisplay
     * @param checked true for saving images, false otherwise
     */
    void ToggledSaveMovie (bool checked);

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
    Foam::Bodies::iterator m_currentTranslatedBody;
    size_t m_currentBody;
    /**
     * Save a jpeg of the current image.
     */
    bool m_saveMovie;
    /**
     * Keeps track of the current frame saved in a file.
     */
    int m_currentFrame;
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

