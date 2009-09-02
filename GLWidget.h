/**
 * @file   GLWidget.h
 * @author Dan R. Lipsa
 *
 * Widget for displaying foam bubbles using OpenGL
 */
#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

#include <vector>
#include <QGLWidget>

class Data;
class Body;
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
    unsigned int GetDisplayedBody ();
    unsigned int GetDisplayedFace ();
    void IncrementDisplayedBody ();
    void DecrementDisplayedBody ();
    void IncrementDisplayedFace ();
    void DecrementDisplayedFace ();


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
    void setRotation (int axis, float angle);
    void displayFacesContour (const std::vector<Body*>& bodies);
    void displayFacesOffset (const std::vector<Body*>& bodies);
    void emitRotationChanged (int axis, int angle);
    static void initLightBodies ();
    static void initLightFlat ();

    ViewType m_viewType;
    std::vector<GLuint> m_object;
    QPoint m_lastPos;
    Data* m_data;
    unsigned int m_displayedBody;
    unsigned int m_displayedFace;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
