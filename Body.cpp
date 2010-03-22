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
#include "ProcessBodyTorus.h"
#include "TriangleFit.h"
#include "EdgeFit.h"

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
    ostream_iterator<OrientedFace*> output (ostr, "\n");
    copy (b.m_faces.begin (), b.m_faces.end (), output);
    ostr << " Body attributes: ";
    return b.PrintAttributes (ostr);
}

AttributesInfo* Body::m_infos;

Body::Body(vector<int>& faceIndexes, vector<Face*>& faces,
	   size_t originalIndex, Data* data,
	   bool duplicate) :
    Element(originalIndex, data, duplicate), m_placedOrientedFaces (0)
{
    using boost::bind;
    m_faces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_faces.begin(), 
               indexToOrientedFace(faces));
    m_normalFaceMap.reset (
	new NormalFaceMap (Vertex::LessThanAngle (m_faces[0]->GetNormal ())));
    BOOST_FOREACH (OrientedFace* of, m_faces)
	m_normalFaceMap->insert (OrientedFace::MakeNormalFacePair (of));
    m_currentNormalFace = m_normalFaceMap->begin ();

    //if (m_data->IsTorus () && m_data->GetSpaceDimension () == 3)
    if (false)
    {
	ProcessBodyTorus<EdgeFit> processForTorus (this);
	processForTorus.Initialize ();
	while (processForTorus.Step ())
	    ;
	processForTorus.End ();
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
    size_t size = m_physicalVertices.size ();
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

bool Body::FitFace (
    const OrientedFace& candidate,
    const OrientedEdge& fitEdge,
    G3D::Vector3* translation)
{
    for (OrientedFace::const_iterator it = candidate.begin ();
	 it != candidate.end (); ++it)
    {
	OrientedEdge candidateEdge = *it;
	if (candidateEdge.fits (fitEdge))
	{
	    *translation = *candidateEdge.GetEdge()->GetBegin () - 
		*fitEdge.GetEdge()->GetBegin ();
	    return true;
	}
    }
    return false;
}



bool Body::FitFace (
    const OrientedFace& candidate,
    const G3D::Vector3* points, size_t pointCount,
    G3D::Vector3* translation)
{
    using G3D::Vector3;
    size_t currentFitPoint = 0;
    bool found = false;

    for (size_t start = 0; 
	 ! found && start < (candidate.size () + pointCount - 1);
	 start++)
    {
	size_t i = start % candidate.size ();
	Vector3 current = *candidate.GetEnd (i);
	switch (currentFitPoint)
	{
	case 0:
	    *translation = (points[0] - current);
	    currentFitPoint++;
	    break;

	case 1:
	    if ((points[1] - current).fuzzyEq(*translation))
	    {
		currentFitPoint++;
		if (pointCount == currentFitPoint)
		    found = true;
	    }
	    else
	    {
		start--;
		currentFitPoint = 0;
	    }
	    break;

	case 2:
	    if ((points[2] - current).fuzzyEq(*translation))	 
		found = true;	    
	    else
	    {
		start--;
		currentFitPoint = 0;
	    }
	    break;
	}
    }
    return found;
}



Body::NormalFaceMap::const_iterator Body::FindNormalFace (
    const G3D::Vector3& normal) const
{
    NormalFaceMap::const_iterator it = m_normalFaceMap->find (normal);
    G3D::Vector3 n = it->first;
    NormalFaceMap::const_iterator prev = it;
    while (n.fuzzyEq (normal) && it != m_normalFaceMap->begin ())
    {
	prev = it;
	--it;	
	n = it->first;
    }
    if (it == m_normalFaceMap->begin () && n.fuzzyEq (normal))
	return it;
    else
	return prev;
}
