/**
 * @file   AdjacentBody.h
 * @author Dan R. Lipsa
 * @date 20 May 2010
 * @brief Keeps track of all bodies a face is part of.
 * @ingroup data model
 */
#ifndef __ADJACENT_BODY_H__
#define __ADJACENT_BODY_H__

class Body;
class OrientedFace;

/**
 * @brief Keeps track of all bodies a face is part of.
 */
class AdjacentBody
{
public:
    AdjacentBody (const boost::shared_ptr<Body>& body, size_t ofIndex) :
	m_body (body), m_orientedFaceIndex (ofIndex)
    {}
    AdjacentBody () :
	m_orientedFaceIndex (0) 
    {}
    boost::shared_ptr<Body> GetBody () const
    {
	return m_body.lock ();
    }
    size_t GetBodyId () const;

    boost::shared_ptr<OrientedFace> GetOrientedFace () const;
    size_t GetOrientedFaceIndex () const
    {
	return m_orientedFaceIndex;
    }
    bool IsOrientedFaceReversed () const;
    string ToString () const;

private:
    boost::weak_ptr<Body> m_body;
    size_t m_orientedFaceIndex;
};

inline ostream& operator<< (ostream& ostr, const AdjacentBody& bi)
{
    return ostr << bi.ToString ();
}


#endif //__ADJACENT_BODY_H__

// Local Variables:
// mode: c++
// End:
