#include<functional>
#include <algorithm>
#include "Body.h"
#include "Element.h"

using namespace std;

Body::~Body()
{
    for_each(m_faces.begin(), m_faces.end(), DeleteElement<OrientedFace>());
}


class IndexToOrientedFace : public unary_function<int, OrientedFace*>
{
public:
    IndexToOrientedFace(vector<Face*>& faces): m_faces(faces) {}

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
    vector<Face*>& m_faces;
};


Body::Body(const vector<int>& face_indexes, vector<Face*>& faces)
{
    transform (face_indexes.begin(), face_indexes.end(), m_faces.begin(), 
	       IndexToOrientedFace(faces));
}
