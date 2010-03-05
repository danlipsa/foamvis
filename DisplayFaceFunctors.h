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
        if (m_widget.GetDisplayedFace () == GLWidget::DISPLAY_ALL ||
	    m_count == m_widget.GetDisplayedFace ())
        {
	    display (of);
	    if (m_count == m_widget.GetDisplayedFace ())
		cdbg << "face " << m_count << ": " << *of << endl;
        }
        m_count++;
    }

    void operator () (const Face* f)
    {
	OrientedFace of(const_cast<Face*>(f), false);
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
    unsigned int m_count;
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
	// specify the normal vector
	const Vertex* begin = f->GetBegin (0);
	const Vertex* end = f->GetEnd (0);
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
	(DisplaySameEdges (m_widget)) (f);
    }
};

#endif //__DISPLAY_FACE_FUNCTORS_H__
