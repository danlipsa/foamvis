#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <QWidget>
#include "ui_Window.h"

class GLWidget;
class Data;
class Window : public QWidget, private Ui::Window
{
    Q_OBJECT

public:
    Window(Data* data);
};


#endif
