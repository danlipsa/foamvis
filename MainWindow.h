/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains the all the user interface
 */
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <vector>
#include <QWidget>
#include "ui_Window.h"

class GLWidget;
class Data;

class MainWindow : public QWidget, private Ui::Window
{
    Q_OBJECT

public:
	MainWindow(std::vector<Data*>& data);
    void keyPressEvent (QKeyEvent* event);
};

#endif //__MAIN_WINDOW_H__

// Local Variables:
// mode: c++
// End:

