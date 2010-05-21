/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Functors that display a face
 */

#ifndef __DISPLAY_FACE_FUNCTORS_H__
#define __DISPLAY_FACE_FUNCTORS_H__

#include "DisplayEdgeFunctors.h"

/**
 * Functor that displays a face
 */
template <typename displayEdges>
class DisplayFace : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the face displayed
     */
    DisplayFace (const GLWidget& widget) : 
    DisplayElement (widget), m_count(0) {}
    /**
     * Functor that displays a face
     * @param f the face to be displayed
     */
    void operator() (const OrientedFace* of)
    {
	size_t faceIndex = m_widget.GetDisplayedFaceIndex ();
/*
	size_t bodyIndex = m_widget.GetDisplayedBodyIndex ();
	size_t bodyId = m_widget.GetDisplayedBodyId ();

        if ((faceIndex == GLWidget::DISPLAY_ALL ||
	     m_count == m_widget.GetDisplayedFaceIndex ()) &&

	    (bodyIndex == GLWidget::DISPLAY_ALL ||
	     of->IsPartOfBody (bodyId)))
*/
        {
	    display (of);
	    if (m_count == faceIndex)
		cdbg << "face " << m_count << ": " << *of << endl;
        }
        m_count++;
    }

    void operator () (const Face* f)
    {
	const OrientedFace of(const_cast<Face*>(f), false);
	operator() (&of);
    }


protected:
    virtual void display (const OrientedFace* of)
    {
	(displayEdges (m_widget)) (of);
    }

private:
    /**
     * Used to display fewer faces (for DEBUG purposes)
     */
    size_t m_count;
};


/**
 * Functor that displays a face using the color specified in the DMP file
 */
class DisplayFaceWithColor : public DisplayFace<DisplaySameEdges>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    DisplayFaceWithColor (const GLWidget& widget) : 
    DisplayFace<DisplaySameEdges> (widget) {}

protected:
    virtual void display (const OrientedFace* of)
    {
	glColor4fv (Color::GetValue(of->GetFace()->GetColor ()));
	(DisplaySameEdges (m_widget)) (of);
    }
};

/**
 * Displays a face and specifies the normal to the face. Used for lighting.
 */
class DisplayFaceWithNormal : public DisplayFace<DisplaySameEdges>
{
public:
    /**
     * Constructor
     * @param widget where to display the face
     */
    DisplayFaceWithNormal (const GLWidget& widget) : 
    DisplayFace<DisplaySameEdges> (widget) {}

protected:
    /**
     * Functor used to display a face together to the normal
     * @param f face to be displayed
     */
    virtual void display (const OrientedFace* f)
    {
	using G3D::Vector3;
	Vector3 normal = f->GetNormal ();
	glNormal (normal);

	// specify the vertices
	(DisplaySameEdges (m_widget)) (f);
    }
};


/**
 * Displays a face and specifies the normal to the face. Used for lighting.
 */
class DisplayFaceVectors : public DisplayFace<DisplaySameEdges>
{
public:
    /**
     * Constructor
     * @param widget where to display the face
     */
    DisplayFaceVectors (const GLWidget& widget) : 
    DisplayFace<DisplaySameEdges> (widget) {}

protected:
    /**
     * Functor used to display a face together to the normal
     * @param f face to be displayed
     */
    virtual void display (const OrientedFace* f)
    {
	using G3D::Vector3;
	OrientedEdge oe;
	f->GetOrientedEdge (0, &oe);
	float size = oe.GetEdgeVector ().length ();
	Vector3 normal = f->GetNormal ();
	Vector3 begin = *oe.GetBegin ();
	glBegin (GL_LINES);
	glVertex (begin);
	glVertex (begin + normal * size);

	for (size_t i = 0; i < f->size (); i++)
	{
	    OrientedEdge oe;
	    f->GetOrientedEdge (i, &oe);
	    Vector3 edgeVector = oe.GetEdgeVector ();
	    Vector3 edgeNormal = edgeVector.cross (normal).unit ();
	    begin = *oe.GetBegin ();
	    glVertex (begin);
	    glVertex (begin + edgeNormal * size);
	}
	glEnd ();

    }
};



#endif //__DISPLAY_FACE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
