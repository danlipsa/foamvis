/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Body class
 */
#include "Body.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "Data.h"
#include "Debug.h"

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
     * Converts a 1-based index into an OrientedFace
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
        i--;
        return new OrientedFace(m_faces[i], reversed);
    }
private:
    /**
     * Vector of Face pointers
     */
    vector<Face*>& m_faces;
};

ostream& operator<< (ostream& ostr, const Body& b)
{
    PrintElements<OrientedFace*> (ostr, b.m_faces, 
				  "faces part of the body", true);
    ostr << " Body attributes: ";
    return b.PrintAttributes (ostr, *Body::m_infos);
}

AttributesInfo* Body::m_infos;

Body::Body(vector<int>& faceIndexes, vector<Face*>& faces,
	   unsigned int originalIndex, Data* data,
	   bool duplicate) :
    Element(originalIndex, data, duplicate)
{
    using boost::bind;
    m_faces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_faces.begin(), 
               indexToOrientedFace(faces));
    if (m_data->IsTorus ())
    {
	NormalFaceMap normalFaceMap;
	BOOST_FOREACH (OrientedFace* f, m_faces)
	    normalFaceMap.insert (
		NormalFaceMap::value_type (
		    f->GetFace ()->GetNormal (), f));
	vector<FaceIntersectionMargin> queue;
	// start with two faces, mark them as placed
	m_faces[0]->SetPlaced (true);
	m_faces[1]->SetPlaced (true);

	// add the two intersection triangles into a queue
	FaceIntersectionMargin firstTriangle, secondTriangle;
	getTrianglesFromFaceIntersection (
	    *m_faces[0], *m_faces[1], &firstTriangle, &secondTriangle);
	queue.push_back (firstTriangle);
	queue.push_back (secondTriangle);
	while (queue.empty ())
	{
	    FaceIntersectionMargin triangle = queue.back ();
	    queue.pop_back ();

	    // place the face that fits over that triangle (this might
	    // create a duplicate face)
	    OrientedFace* face = fitFace (triangle, normalFaceMap);

	    // if the face was not placed before
	       // add two more angles in the queue
	    if (! face->IsPlaced ())
	    {
		face->SetPlaced (true);
		getTrianglesFromFaceIntersection (
		    *face, *triangle.m_first.m_face, 
		    &firstTriangle, &secondTriangle);
		queue.push_back (secondTriangle);
		getTrianglesFromFaceIntersection (
		    *face, *triangle.m_second.m_face, 
		    &firstTriangle, &secondTriangle);
		queue.push_back (firstTriangle);
	    }
	}
	for_each (m_faces.begin (), m_faces.end (),
		  bind (&OrientedFace::SetPlaced, _1, false));
    }
}

Body::~Body()
{
    using boost::bind;
    for_each(m_faces.begin(), m_faces.end(), 
	     bind (DeletePointer<OrientedFace>(), _1));
}

void Body::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::LAGRANGE_MULTIPLIER),
        new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLUME),
        new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLCONST),
        new RealAttributeCreator());
}

/**
 * Functor that caches an edge and its vertices
 */
class cacheEdgeVertices
{
public:
    /**
     * Constructor
     * @param body where to cache the edge and vertices
     */
    cacheEdgeVertices (Body& body) : m_body (body) {}
    /**
     * Functor that caches an edge and its vertices
     * @param oe the edge to cache
     */
    void operator () (OrientedEdge* oe)
    {
	Edge *e = oe->GetEdge ();
	m_body.CacheEdge (e);
	m_body.CacheVertex (e->GetBegin ());
	m_body.CacheVertex (e->GetEnd ());
    }
private:
    /**
     * Where to cache the edge and vertices
     */
    Body& m_body;
};

/**
 * Functor that caches edges and vertices in vectors stored in the Body
 */
class cacheEdgesVertices
{
public:
    /**
     * Constructor
     * @param body object we work on
     */
    cacheEdgesVertices (Body& body) : m_body(body) {}
    /**
     * Functor that caches edges and vertices in vectors stored in the Body
     * @param of cache all edges and vertices for this OrientedFace
     */
    void operator() (OrientedFace* of)
    {
	vector<OrientedEdge*> oev = of->GetFace ()->GetOrientedEdges ();
	for_each (oev.begin (), oev.end (), cacheEdgeVertices (m_body));
    }
private:
    /**
     * Where to we store edges and vertices
     */
    Body& m_body;
};


void Body::CacheEdgesVertices ()
{
    for_each (m_faces.begin (), m_faces.end(), cacheEdgesVertices(*this));
    split (m_vertices, m_tessellationVertices, m_physicalVertices);
    split (m_edges, m_tessellationEdges, m_physicalEdges);
}

template <typename T>
void Body::split (
    set<T*>& src,
    vector<T*>& destTessellation,
    vector<T*>& destPhysical)
{
    destTessellation.resize (src.size ());
    copy (src.begin (), src.end (), destTessellation.begin ());
    typename vector<T*>::iterator bp;
    bp = partition (destTessellation.begin (),destTessellation.end (), 
		    not1(mem_fun(&T::IsPhysical)));
    destPhysical.resize (destTessellation.end () - bp);
    copy (bp, destTessellation.end (), 
	  destPhysical.begin ());
    destTessellation.resize (
	bp - destTessellation.begin ());
}


void Body::CalculateCenter ()
{
    using G3D::Vector3;
    unsigned int size = m_physicalVertices.size ();
    RuntimeAssert (
	size != 0, 
	"Call Body::CacheEdgesVertices before calling this function");
    m_center = accumulate (
	m_physicalVertices.begin (), m_physicalVertices.end (), m_center, 
	&Vertex::Accumulate);
    m_center /= Vector3(size, size, size);
}

void Body::UpdateFacesAdjacency ()
{
    using boost::bind;
    vector<OrientedFace*>& of = GetOrientedFaces ();
    for_each (of.begin (), of.end (),
	      bind(&OrientedFace::AddAdjacentBody, _1, this));
}

void Body::getTrianglesFromFaceIntersection (
    const OrientedFace& firstFace, const OrientedFace& secondFace,
    FaceIntersectionMargin* firstTriangle, FaceIntersectionMargin* secondTriangle)
{
    for (size_t i = 0; i < firstFace.GetEdgeCount (); i++)
    {
	for (size_t j = 0; j < secondFace.GetEdgeCount (); j++)
	{
	    if (*(firstFace.GetOrientedEdge (i)->GetEdge ()) == 
		*(secondFace.GetOrientedEdge (j)->GetEdge ()))
	    {
		firstTriangle->m_first.m_face = &firstFace;
		firstTriangle->m_first.m_edgeIndex = i;
		firstTriangle->m_second.m_face = &secondFace;
		firstTriangle->m_second.m_edgeIndex = j;
		firstTriangle->m_margin = FaceIntersectionMargin::BEFORE_FIRST_AFTER_SECOND;
		*secondTriangle = *firstTriangle;
		secondTriangle->m_margin = FaceIntersectionMargin::AFTER_FIRST_BEFORE_SECOND;
	    }
	}
    }
}


void Body::FaceIntersectionMargin::GetTriangle (
    const FaceEdgeIndex& first, const FaceEdgeIndex& second,
    boost::array<G3D::Vector3, 3>* t)
{
    boost::array<G3D::Vector3, 3>& triangle = *t;
    size_t afterIndex = second.m_face->GetNextValidIndex (
	second.m_edgeIndex);
    triangle[0] = *second.m_face->GetEnd (afterIndex);
    triangle[1] = *second.m_face->GetBegin (afterIndex);
    size_t beforeIndex = first.m_face->GetPreviousValidIndex (
	first.m_edgeIndex);
    G3D::Vector3 otherTwo = *first.m_face->GetEnd (beforeIndex);
    RuntimeAssert (triangle[1] == otherTwo,
		   "Invalid triangle: ", triangle[1], ", ", otherTwo);
    triangle[2] = *first.m_face->GetBegin (beforeIndex);
}


OrientedFace* Body::fitFace (
    const FaceIntersectionMargin& faceIntersectionMargin, 
    const NormalFaceMap& normalFaceMap)
{
    using G3D::Vector3;
    boost::array<Vector3, 3> triangle;
    if (faceIntersectionMargin.m_margin == 
	FaceIntersectionMargin::BEFORE_FIRST_AFTER_SECOND)
	FaceIntersectionMargin::GetTriangle (
	    faceIntersectionMargin.m_first, 
	    faceIntersectionMargin.m_second, &triangle);
    else
	FaceIntersectionMargin::GetTriangle (
	    faceIntersectionMargin.m_second,
	    faceIntersectionMargin.m_first, &triangle);
    Vector3 normal = (triangle[1] - triangle[0]).cross (
	triangle[2] - triangle[1]);
    pair<NormalFaceMap::const_iterator, NormalFaceMap::const_iterator> range = 
	normalFaceMap.equal_range (normal);
    for (NormalFaceMap::const_iterator it = range.first;
	 it != range.second; ++it)
    {
	OrientedFace* face = fitFace ((*it).second, triangle);
	if (face != 0)
	    return face;
    }
}

OrientedFace* Body::fitFace (
    OrientedFace* face, const boost::array<G3D::Vector3, 3>& triangle)
{
    using G3D::Vector3;
    size_t currentFitPoint = 0;
    bool found = false;

    Vector3 translate;
    for (size_t i = 0; ! found && i < face->GetEdgeCount (); i++)
    {
	Vector3 current = *face->GetEnd (i);
	switch (currentFitPoint)
	{
	case 0:
	    translate = (triangle[currentFitPoint] - current);
	    currentFitPoint++;
	    break;
	case 1:
	    if ((triangle[currentFitPoint] - current).fuzzyEq(translate))
		currentFitPoint++;
	    else
	    {
		i--;
		currentFitPoint = 0;
	    }
	    break;
	case 2:
	    if ((triangle[currentFitPoint] - current).fuzzyEq(translate))
	    {
		found = true;
		//found a fit, we might need a translation
		face->SetFace (
		    m_data->GetFaceDuplicate (
			*face->GetFace (),
			*(face->GetOrientedEdge(0)->GetBegin ())));
		break;
	    }
	}
    }
    RuntimeAssert (found, "Cannot fit face");
    return face;
}

