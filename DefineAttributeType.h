/**
 * @file   DefineAttributeType.h
 * @author Dan R. Lipsa
 *
 * Possible types for attributes defined in a DMP file
 */
#ifndef __DEFINE_ATTRIBUTE_TYPE_H__
#define __DEFINE_ATTRIBUTE_TYPE_H__

/**
 * Types of  attributes defined in a  Surface Evolver DMP  file. Use a
 * struct to enclose the enum.
 */
struct DefineAttribute
{
    /**
     * Types of attributes defined in a Surface Evolver DMP file
     */
    enum Type {
        VERTEX,
        EDGE,
        FACE,
        BODY,
        COUNT
    };
};

#endif //__DEFINE_ATTRIBUTE_TYPE_H__

// Local Variables:
// mode: c++
// End:
