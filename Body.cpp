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
#include "ElementUtils.h"
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
    void operator() (OrientedFace* of)
    {
	Face::OrientedEdges& oev = of->GetFace ()->GetOrientedEdges ();
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

// Methods
// ======================================================================

Body::Body(vector<int>& faceIndexes, vector<Face*>& faces,
	   size_t id, ElementStatus::Name status) :
    Element(id, status)
{
    using boost::bind;
    m_orientedFaces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_orientedFaces.begin(), 
               indexToOrientedFace(faces));
    m_normalFaceMap.reset (
	new NormalFaceMap (
	    VectorLessThanAngle (m_orientedFaces[0]->GetNormal ())));
    BOOST_FOREACH (OrientedFace* of, m_orientedFaces)
	m_normalFaceMap->insert (OrientedFace::MakeNormalFacePair (of));
    m_currentNormalFace = m_normalFaceMap->begin ();
}

Body::~Body ()
{
    for_each(m_orientedFaces.begin(), m_orientedFaces.end(), bl::delete_ptr ());
}

void Body::CacheEdgesVertices ()
{
    for_each (m_orientedFaces.begin (), m_orientedFaces.end(),
	      cacheEdges(*this));
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
    bp = partition (destTessellation.begin (), destTessellation.end (), 
		    not1(mem_fun(&T::IsPhysical)));
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
	&Vertex::Accumulate);
    m_center /= Vector3(size, size, size);
}


ostream& Body::PrintFaceInformation (ostream& ostr) const
{
    size_t bodyId = GetId ();
    ostr << "Face edge information for body :" << bodyId << endl;
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	Face* f = m_orientedFaces[i]->GetFace ();
	ostr << "Face " << f->GetStringId () 
	     << " part of bodies: ";
	for (size_t j = 0; j < f->GetBodyPartOfSize (); j++)
	{
	    const BodyIndex& bi = f->GetBodyPartOf (j);
	    ostr << setw (3) << bi.GetBody ()->GetId ()
		 << " at index " << bi.GetOrientedFaceIndex () << " ";
	}
	ostr << endl;
    }
    return ostr;
}



ostream& Body::PrintEdgeInformation (ostream& ostr) const
{
    size_t bodyId = GetId ();
    ostr << "Face edge information for body :" << bodyId << endl;
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	OrientedFace* of = m_orientedFaces[i];
	const BodyIndex& bi = of->GetBodyPartOf ();
	ostr << "Face " << of->GetStringId () 
	     << " part of body " << setw (3) << bi.GetBody ()->GetId ()
	     << " at index " << bi.GetOrientedFaceIndex () << endl;
	for (size_t j = 0; j < of->size (); j++)
	{
	    const OrientedEdge& oe = of->GetOrientedEdge (j);
	    ostr << "    Edge " << oe.GetStringId () << " is part of: ";
	    for (size_t k = 0; k < oe.GetFacePartOfSize (); k++)
	    {
		const OrientedFaceIndex& ofi = oe.GetFacePartOf (k);
		if (ofi.GetBodyId () == bodyId &&
		    oe.IsReversed () == ofi.IsOrientedEdgeReversed ())
		    ostr << ofi << " ";
	    }
	    ostr << endl;
	}
    }
    return ostr;
}

void Body::UpdatePartOf ()
{
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	OrientedFace* of = m_orientedFaces[i];
	of->AddBodyPartOf (this, i);
	of->UpdateFacePartOf ();
    }
    PrintFaceInformation (cdbg);
}

void Body::ClearPartOf ()
{
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	OrientedFace* of = m_orientedFaces[i];
	of->ClearBodyPartOf ();
	of->ClearFacePartOf ();
    }
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

void Body::PrintDomains (ostream& ostr) const
{
    Vertex::PrintDomains (ostr, m_vertices);
}


bool Body::HasWrap () const
{
    BOOST_FOREACH (OrientedFace* of, m_orientedFaces)
	if (of->GetFace ()->HasWrap ())
	    return true;
    return false;
}

Face* Body::GetFace (size_t i) const
{
    return GetOrientedFace (i)->GetFace ();
}

void Body::Unwrap (Foam* foam)
{
    ProcessBodyTorus(foam, this).Unwrap ();
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
    ostream_iterator<OrientedFace*> output (ostr, "\n");
    copy (b.m_orientedFaces.begin (), b.m_orientedFaces.end (), output);
    ostr << "Body attributes: ";
    b.PrintAttributes (ostr);
    ostr << "\nBody center: " << b.m_center;
				 return ostr;
}
