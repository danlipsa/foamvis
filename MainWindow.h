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
class Data;

/**
 * Class that contains the OpenGL widget and all other UI.
 */
class MainWindow : public QWidget, private Ui::Window
{
    Q_OBJECT
    
public:
    MainWindow(std::vector<Data*>& data);
    void keyPressEvent (QKeyEvent* event);

public Q_SLOTS:
    void TooglePlay ();
    void BeginSlider ();
    void EndSlider ();
    void IncrementSlider ();
    void SliderValueChanged (int value);

private:
    void enableBegin (bool enable);
    void enableEnd (bool enable);
    void enablePlay (bool enable);
    void updateButtons ();

    QTimer* m_timer;
    bool m_play;
    const char* PLAY_TEXT;
    const char* PAUSE_TEXT;
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

