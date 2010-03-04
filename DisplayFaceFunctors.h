/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Functors that display a face
 */

#ifndef __DISPLAY_FACE_FUNCTORS_H__
#define __DISPLAY_FACE_FUNCTORS_H__

#include "DisplayFace.h"
#include "DisplayEdgeFunctors.h"

/**
 * Functor that displays a face
 */
template <typename displayEdges = displaySameEdges>
class displayFace : public DisplayFace
{
public:
    /**
     * Constructor
     * @param widget Where should be the face displayed
     */
    displayFace (const GLWidget& widget) : 
        DisplayFace (widget), m_count(0) {}
    /**
     * Functor that displays a face
     * @param f the face to be displayed
     */
    virtual void operator() (OrientedFace* f)
    {
	operator() (f->GetFace ());
    }

    void operator () (Face* f)
    {
        if (m_widget.GetDisplayedFace () == GLWidget::DISPLAY_ALL ||
	    m_count == m_widget.GetDisplayedFace ())
        {
            (displayEdges (m_widget)) (f);
            if (m_count == m_widget.GetDisplayedFace ())
                cdbg << "face " << m_count << ": " << *f << endl;
        }
        m_count++;
    }

protected:
    /**
     * Used to display fewer faces (for DEBUG purposes)
     */
    unsigned int m_count;
};


/**
 * Functor that displays a face using the color specified in the DMP file
 */
class displayFaceWithColor : public displayFace<>
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    displayFaceWithColor (GLWidget& widget) : 
        displayFace<> (widget) {}

    /**
     * Functor that displays a colored face
     * @param f face to be displayed
     */
    virtual void operator() (OrientedFace* f)
    {
	operator() (f->GetFace ());
    }

    void operator () (Face* f)
    {
        if (m_count <= m_widget.GetDisplayedFace ())
        {
            glColor4fv (Color::GetValue(f->GetColor ()));
            (displaySameEdges (m_widget)) (f);
        }
        m_count++;
    }
};

/**
 * Displays a face and specifies the normal to the face. Used for lighting.
 */
class displayFaceWithNormal : public displayFace<>
{
public:
    /**
     * Constructor
     * @param widget where to display the face
     */
    displayFaceWithNormal (GLWidget& widget) : 
        displayFace<> (widget) {}
    /**
     * Functor used to display a face together to the normal
     * @param f face to be displayed
     */
    virtual void operator() (OrientedFace* f)
    {
        if (m_count <= m_widget.GetDisplayedFace ())
        {
	    using G3D::Vector3;
	    // specify the normal vector
	    Vertex* begin = f->GetBegin (0);
	    Vertex* end = f->GetEnd (0);
	    Vector3 first(end->x - begin->x,
			  end->y - begin->y,
			  end->z - begin->z);
	    begin = f->GetBegin (1);
	    end = f->GetEnd (1);
	    Vector3 second(end->x - begin->x,
			   end->y - begin->y,
			   end->z - begin->z);
	    Vector3 normal (first.cross(second).unit ());
	    glNormal3f (normal.x, normal.y, normal.z);

	    // specify the vertices
	    (displaySameEdges (m_widget)) (f);
	}
	m_count++;
    }
};

#endif //__DISPLAY_FACE_FUNCTORS_H__
