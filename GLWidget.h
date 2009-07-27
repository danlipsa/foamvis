#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

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
    GLuint makeVertices ();
    GLuint makeEdges ();
    GLuint makeFacets ();
    void normalizeAngle(int *angle);

    GLuint m_objectVertices;
    GLuint m_objectEdges;
    GLuint m_objectFacets;

    int m_xRot;
    int m_yRot;
    int m_zRot;
    QPoint m_lastPos;
    Data& m_data;
};

#endif
