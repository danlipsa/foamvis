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
class OrientedFace;

class BodyIndex
{
public:
    BodyIndex (boost::shared_ptr<Body> body, size_t ofIndex) :
	m_body (body), m_orientedFaceIndex (ofIndex)
    {}
    BodyIndex () :
	m_orientedFaceIndex (0) 
    {}
    boost::shared_ptr<Body> GetBody () const
    {
	return m_body;
    }
    size_t GetBodyId () const;

    boost::shared_ptr<OrientedFace> GetOrientedFace () const;
    size_t GetOrientedFaceIndex () const
    {
	return m_orientedFaceIndex;
    }
    bool IsOrientedFaceReversed () const;

public:
    friend ostream& operator<< (ostream& ostr, const BodyIndex& bi);

private:
    boost::shared_ptr<Body> m_body;
    size_t m_orientedFaceIndex;
};

#endif //__BODY_INDEX_H__

// Local Variables:
// mode: c++
// End:
