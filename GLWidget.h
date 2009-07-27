#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

class Data;
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(Data& data, QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    GLuint makeObject();
    void quad(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2,
              GLdouble x3, GLdouble y3, GLdouble x4, GLdouble y4);
    void normalizeAngle(int *angle);

    GLuint m_object;
    int m_xRot;
    int m_yRot;
    int m_zRot;
    QPoint m_lastPos;
    Data& m_data;
};

#endif
