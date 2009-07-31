#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__

class Attribute
{
public:
    virtual ~Attribute () {}
    enum Type {
	VERTEX_TYPE,
	EDGE_TYPE,
	FACE_TYPE,
	BODY_TYPE,
	ATTRIBUTE_TYPE_COUNT
    };
};


#endif
// Local Variables:
// mode: c++
// End:
