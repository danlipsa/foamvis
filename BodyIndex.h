/**
 * @file   BodyIndex.h
 * @author Dan R. Lipsa
 * @date 20 May 2010
 *
 * Declaration of BodyIndex class
 */
#ifndef __BODY_INDEX_H__
#define __BODY_INDEX_H__

class Body;

struct BodyIndex
{
    BodyIndex (Body* body, size_t ofIndex) :
	m_body (body), m_orientedFaceIndex (ofIndex)
    {}
    BodyIndex () :
	m_body(0), m_orientedFaceIndex (0) 
    {}
    Body* m_body;
    size_t m_orientedFaceIndex;
};

#endif //__BODY_INDEX_H__

// Local Variables:
// mode: c++
// End:
