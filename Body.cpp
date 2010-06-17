/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Body class
 */
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Body.h"
#include "Edge.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Utils.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "ParsingDriver.h"
#include "ProcessBodyTorus.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

/**
 * Functor that caches an edge and its vertices
 */
class cacheEdge
{
public:
    /**
     * Constructor
     * @param body where to cache the edge and vertices
     */
    cacheEdge (Body& body) : m_body (body) {}
    /**
     * Functor that caches an edge and its vertices
     * @param oe the edge to cache
     */
    void operator () (boost::shared_ptr<OrientedEdge> oe)
    {
	boost::shared_ptr<Edge> e = oe->GetEdge ();
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
class cacheEdges
{
public:
    /**
     * Constructor
     * @param body object we work on
     */
    cacheEdges (Body& body) : m_body(body) {}
    /**
     * Functor that caches edges and vertices in vectors stored in the Body
     * @param of cache all edges and vertices for this OrientedFace
     */
    void operator() (boost::shared_ptr<OrientedFace> of)
    {
	Face::OrientedEdges& oev = of->GetFace ()->GetOrientedEdges ();
	for_each (oev.begin (), oev.end (), cacheEdge (m_body));
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
class indexToOrientedFace : 
    public unary_function<int, boost::shared_ptr<OrientedFace> >
{
public:
    /**
     * Constructor
     * @param faces vector of Face pointers. This is where the indexes point to.
     */
    indexToOrientedFace(const vector< boost::shared_ptr<Face> >& faces): 
	m_faces(faces) {}
    /**
     * Converts a 1-based index into an OrientedFace
     * @param i index into a vector of Face pointers
     * @return an OrientedFace pointer
     */
    boost::shared_ptr<OrientedFace>  operator() (int i)
    {
        bool reversed = false;
        if (i < 0)
        {
            i = -i;
            reversed = true;
        }
        i--;
        return boost::make_shared<OrientedFace> (m_faces[i], reversed);
    }
private:
    /**
     * Vector of Face pointers
     */
    const vector<boost::shared_ptr<Face> >& m_faces;
};

// Private functions
// ======================================================================
/**
 * Accumulates along X, Y or Z
 * @param result where we accumulate
 * @param v the vertex
 * @return result + the value of the vertex along the specified axis
 */
G3D::Vector3 VertexAccumulate (G3D::Vector3 result,
			       const boost::shared_ptr<Vertex>& v)
{
    return result + *v;
}


// Methods
// ======================================================================

Body::Body(
    const vector<int>& faceIndexes,
    const vector<boost::shared_ptr<Face> >& faces,
    size_t id, ElementStatus::Duplicate duplicateStatus) :
    Element(id, duplicateStatus)
{
    m_orientedFaces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_orientedFaces.begin(), 
               indexToOrientedFace(faces));
}

void Body::CacheEdgesVertices (size_t dimension, bool isQuadratic)
{
    m_edges.clear ();
    m_vertices.clear ();
    m_physicalVertices.clear ();
    m_tessellationVertices.clear ();
    m_physicalEdges.clear ();
    m_tessellationEdges.clear ();

    for_each (m_orientedFaces.begin (), m_orientedFaces.end(),
	      cacheEdges(*this));
    split (m_vertices, m_tessellationVertices, m_physicalVertices, 
	   dimension, isQuadratic);
    split (m_edges, m_tessellationEdges, m_physicalEdges,
	   dimension, isQuadratic);
}

template <typename T>
void Body::split (
    set< boost::shared_ptr<T> >& src,
    vector< boost::shared_ptr<T> >& destTessellation,
    vector< boost::shared_ptr<T> >& destPhysical,
    size_t dimension, bool isQuadratic)
{
    destTessellation.resize (src.size ());
    copy (src.begin (), src.end (), destTessellation.begin ());
    typename vector< boost::shared_ptr<T> >::iterator bp;
    bp = partition (destTessellation.begin (), destTessellation.end (), 
		    !boost::bind(&T::IsPhysical, _1, 
				dimension, isQuadratic));
    destPhysical.resize (destTessellation.end () - bp);
    copy (bp, destTessellation.end (), destPhysical.begin ());
    destTessellation.resize (bp - destTessellation.begin ());
}


void Body::CalculateCenter ()
{
    using G3D::Vector3;
    size_t size = m_physicalVertices.size ();
    RuntimeAssert (
	size != 0, 
	"There are no physical vertices in this body. Call"
	" Body::CacheEdgesVertices before calling this function");
    m_center = accumulate (
	m_physicalVertices.begin (), m_physicalVertices.end (), m_center, 
	&VertexAccumulate);
    m_center /= Vector3(size, size, size);
}


void Body::UpdatePartOf (const boost::shared_ptr<Body>& body)
{
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	boost::shared_ptr<OrientedFace>  of = m_orientedFaces[i];
	of->AddBodyPartOf (body, i);
	of->UpdateFacePartOf (of);
    }
}



void Body::PrintDomains (ostream& ostr) const
{
    Vertex::PrintDomains (ostr, m_vertices);
}


bool Body::HasWrap () const
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace>  of, m_orientedFaces)
	if (of->GetFace ()->HasWrap ())
	    return true;
    return false;
}

boost::shared_ptr<Face>  Body::GetFace (size_t i) const
{
    return GetOrientedFace (i)->GetFace ();
}

void Body::Unwrap (Foam* foam)
{
    ProcessBodyTorus(foam, this).Unwrap ();
}

void Body::Print ()
{
    cout << *this;
}


// Static and Friends Methods
// ======================================================================

void Body::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    auto_ptr<AttributeCreator> ac (new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);
    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::LAGRANGE_MULTIPLIER), ac);
    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLUME), ac);
    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLCONST), ac);
}

ostream& operator<< (ostream& ostr, const Body& b)
{
    ostr << "Body " << b.GetId () << ":" << endl;
    ostream_iterator<boost::shared_ptr<OrientedFace> > output (ostr, "\n");
    copy (b.m_orientedFaces.begin (), b.m_orientedFaces.end (), output);
    ostr << "Body attributes: ";
    b.PrintAttributes (ostr);
    return ostr << "\nBody center: " << b.m_center;
}
