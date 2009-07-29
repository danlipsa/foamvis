#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

#include <QGLWidget>

class Data;
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void SetData (Data* data) 
    {
	m_data = data;
    }

public slots:
    void SetXRotation(int angle);
    void SetYRotation(int angle);
    void SetZRotation(int angle);

signals:
    void XRotationChanged(int angle);
    void YRotationChanged(int angle);
    void ZRotationChanged(int angle);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    GLuint makeVertices ();
    GLuint makeEdges ();
    GLuint makeFaces ();
    GLuint makeBodies ();
    void normalizeAngle(int *angle);

    GLuint m_objectVertices;
    GLuint m_objectEdges;
    GLuint m_objectFaces;
    GLuint m_objectBodies;

    int m_xRot;
    int m_yRot;
    int m_zRot;
    QPoint m_lastPos;
    Data* m_data;
};

#endif
