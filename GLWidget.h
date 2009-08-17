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
    unsigned int GetFacesDisplayed ();
    void IncrementFacesDisplayed ();
    void DecrementFacesDisplayed ();

public Q_SLOTS:
    void ViewVertices (bool checked);
    void ViewEdges (bool checked);
    void ViewFaces (bool checked);
    void ViewBodies (bool checked);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
        enum ViewType {
        VERTICES,
        EDGES,
        FACES,
        BODIES,
        VIEW_TYPE_NUMBER
    };

    GLuint displayVertices ()
    {return displayEV (VERTICES);}
    GLuint displayEdges ()
    {return displayEV (EDGES);}
    GLuint displayEV (ViewType type);
    GLuint displayFaces ();
    GLuint displayBodies ();
    void setRotation (int axis, int angle);
    void setAccumulator (int axis, float value) {m_accumulator[axis] = value;}
    float getAccumulator (int axis) {return m_accumulator[axis];}
    void emitRotationChanged (int axis, int angle);
    void accumulate (int axis, float value);
    static void initLightBodies ();
    static void initLightFlat ();

    ViewType m_viewType;
    vector<GLuint> m_object;
    vector<float> m_accumulator;
    QPoint m_lastPos;
    Data* m_data;
    unsigned int m_facesDisplayed;
    bool m_debug;
};

#endif

// Local Variables:
// mode: c++
// End:
