/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Body class
 */
#include<functional>
#include <algorithm>
#include "Body.h"
#include "Element.h"

/**
 * STL unary  function that converts a  signed index into  a vector of
 * Face  objects into a  OrientedFace object.  A negative  index means
 * that the Face object is listed  in reverse order in the Body object
 * than in the vector of Face objects.
 */
class indexToOrientedFace : public unary_function<int, OrientedFace*>
{
public:
    /**
     * Constructor
     * @param faces vector of Face pointers. This is where the indexes point to.
     */
    indexToOrientedFace(vector<Face*>& faces): m_faces(faces) {}
    /**
     * Converts a signed integer into a OrientedFace
     * @param i index into a vector of Face pointers
     * @return an OrientedFace pointer
     */
    OrientedFace* operator() (int i)
    {
	bool reversed = false;
	if (i < 0)
	{
	    i = -i;
	    reversed = true;
	}
	return new OrientedFace(m_faces[i], reversed);
    }
private:
    /**
     * Vector of Face pointers
     */
    vector<Face*>& m_faces;
};



Body::~Body()
{
    for_each(m_faces.begin(), m_faces.end(), DeleteElementPtr<OrientedFace>);
}

Body::Body(const vector<int>& faceIndexes, vector<Face*>& faces)
{
    m_faces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_faces.begin(), 
	       indexToOrientedFace(faces));
}

ostream& operator<< (ostream& ostr, Body& b)
{
    if (&b == 0)
	ostr << "NULL";
    else
	PrintElementPtrs<OrientedFace> (ostr, b.m_faces, 
					"faces part of the body", true);
    return ostr;
}
