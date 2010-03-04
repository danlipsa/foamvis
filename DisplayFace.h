/**
 * @file   DisplayFace.h
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Interface for functors that display a face
 */

#ifndef __DISPLAY_FACE_H__
#define __DISPLAY_FACE_H__

class DisplayFace
{
public:
    DisplayFace (const GLWidget& widget) : m_widget (widget) {}
    virtual void operator() (OrientedFace*) {};
protected:
    const GLWidget& m_widget;
};


#endif //__DISPLAY_FACE_H__
