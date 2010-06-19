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
    cacheEdge (set< boost::shared_ptr<Vertex> >* vertices) :
	m_vertices (vertices)
    {
    }
    /**
     * Functor that caches an edge and its vertices
     * @param oe the edge to cache
     */
    void operator () (const boost::shared_ptr<OrientedEdge>& oe)
    {
	boost::shared_ptr<Edge> e = oe->GetEdge ();
	m_vertices->insert (e->GetBegin ());
	m_vertices->insert (e->GetEnd ());
    }
private:
    /**
     * Where to cache the edge and vertices
     */
    set< boost::shared_ptr<Vertex> >* m_vertices;
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
    cacheEdges (set< boost::shared_ptr<Vertex> >* vertices) :
	m_vertices(vertices) 
    {
    }
    /**
     * Functor that caches edges and vertices in vectors stored in the Body
     * @param of cache all edges and vertices for this OrientedFace
     */
    void operator() (const boost::shared_ptr<OrientedFace>& of)
    {
	Face::OrientedEdges& oev = of->GetFace ()->GetOrientedEdges ();
	for_each (oev.begin (), oev.end (), cacheEdge (m_vertices));
    }
private:
    /**
     * Where to we store edges and vertices
     */
    set< boost::shared_ptr<Vertex> >* m_vertices;
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
    boost::shared_ptr<OrientedFace> operator() (int i)
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

void Body::cacheBodyEdges (size_t dimension, bool isQuadratic,
			   vector< boost::shared_ptr<Vertex> >* physicalVertices)
{
    set< boost::shared_ptr<Vertex> > vertices;
    vector< boost::shared_ptr<Vertex> > tessellationVertices;

    cacheEdges c(&vertices);
    for_each (m_orientedFaces.begin (), m_orientedFaces.end(), c);
    split (vertices, tessellationVertices, *physicalVertices, 
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


void Body::CalculateCenter (size_t dimension, bool isQuadratic)
{
    using G3D::Vector3;
    vector< boost::shared_ptr<Vertex> > physicalVertices;
    cacheBodyEdges (dimension, isQuadratic, &physicalVertices);
    size_t size = physicalVertices.size ();
    RuntimeAssert (
	size != 0, 
	"There are no physical vertices in this body. Call"
	" Body::CacheEdgesVertices before calling this function");
    m_center = accumulate (
	physicalVertices.begin (), physicalVertices.end (),
	G3D::Vector3::zero (), &VertexAccumulate);
    m_center /= Vector3(size, size, size);
}


void Body::UpdatePartOf (const boost::shared_ptr<Body>& body)
{
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	boost::shared_ptr<OrientedFace> of = m_orientedFaces[i];
	of->AddBodyPartOf (body, i);
	of->UpdateFacePartOf (of);
    }
}


bool Body::HasWrap () const
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, m_orientedFaces)
	if (of->GetFace ()->HasWrap ())
	    return true;
    return false;
}

boost::shared_ptr<Face> Body::GetFace (size_t i) const
{
    return GetOrientedFace (i)->GetFace ();
}

void Body::Unwrap (Foam* foam)
{
    ProcessBodyTorus(foam, this).Unwrap ();
}

string Body::ToString () const
{
    ostringstream ostr;
    ostr << "Body " << GetId () << ":" << endl;
    ostr << m_orientedFaces.size () << " faces part of the body\n";
    ostream_iterator<boost::shared_ptr<OrientedFace> > ofOutput (ostr, "\n");
    copy (m_orientedFaces.begin (), m_orientedFaces.end (), ofOutput);
    ostr << "Body attributes: ";
    PrintAttributes (ostr);
    ostr << "\nBody center: " << m_center;
    return ostr.str ();
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

