#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QWidget>
#include "ui_Window.h"

class GLWidget;
class Data;

class MainWindow : public QWidget, private Ui::Window
{
    Q_OBJECT

public:
    MainWindow(Data* data);
    void keyPressEvent (QKeyEvent* event);
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

