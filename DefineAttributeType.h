#ifndef __DEFINE_ATTRIBUTE_TYPE_H__
#define __DEFINE_ATTRIBUTE_TYPE_H__

struct DefineAttribute
{
    enum Type {
        VERTEX,
        EDGE,
        FACE,
        BODY,
        COUNT
    };
};

#endif //__DEFINE_ATTRIBUTE_TYPE_H__
