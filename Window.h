#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

class GLWidget;
class Data;
class Window : public QWidget
{
    Q_OBJECT

public:
    Window(Data& data);

private:
    QSlider *createSlider();

    GLWidget *m_glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
};


#endif
