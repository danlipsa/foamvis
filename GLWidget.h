#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

#include <vector>
#include <QGLWidget>

using namespace std;
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
    void ViewVertices (bool checked);
    void ViewEdges (bool checked);
    void ViewFaces (bool checked);
    void ViewBodies (bool checked);

signals:
    void XRotationChanged(int angle);
    void YRotationChanged(int angle);
    void ZRotationChanged(int angle);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    GLuint displayVertices ();
    GLuint displayEdges ();
    GLuint displayFaces ();
    GLuint displayBodies ();
    void normalizeAngle(int *angle);
    void setRotation (int axis, int angle) {m_rotation[axis] = angle;}
    void setRotationSlot (int axis, int angle);
    int getRotation (int axis) {return m_rotation[axis];}
    void setAccumulator (int axis, float value) {m_accumulator[axis] = value;}
    float getAccumulator (int axis) {return m_accumulator[axis];}
    void emitRotationChanged (int axis, int angle);
    void accumulate (int axis, float value);
    static void initLightBodies ();
    static void initLightFlat ();

    enum ViewType {
	VERTICES,
	EDGES,
	FACES,
	BODIES,
	VIEW_TYPE_NUMBER
    } m_viewType;
    vector<GLuint> m_object;
    vector<int> m_rotation;
    vector<float> m_accumulator;
    QPoint m_lastPos;
    Data* m_data;
};

#endif
