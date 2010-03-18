/**
 * @file   DisplayFaceInterface.cpp
 * @author Dan R. Lipsa
 * @date  3 March 2010
 *
 * Interface for functors that display a face
 */

#ifndef __DISPLAY_FACE_H__
#define __DISPLAY_FACE_H__

#include "DisplayFaceInterface.h"

class DisplayFaceInterface : public DisplayElement
{
public:
    DisplayFaceInterface (const GLWidget& widget) : DisplayElement (widget) {}
    virtual void operator() (const OrientedFace*)
    {
    }
};
