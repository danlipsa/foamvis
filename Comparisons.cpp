/**
 * @file   Comparisons.cpp
 * @author Dan R. Lipsa
 * @date 25 March 2010
 *
 * Implementation comparison functors
 */
#include "Body.h"
#include "Comparisons.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "OrientedFace.h"
#include "AdjacentOrientedFace.h"




// ======================================================================
// VertexLessThan

bool VertexLessThan::operator () (
    const boost::shared_ptr<const Vertex>& first,
    const boost::shared_ptr<const Vertex>& second) const
{
    return *first < *second;
}


// ======================================================================
// EdgeLessThan

bool EdgeLessThan::operator () (
    const boost::shared_ptr<const Edge>& first,
    const boost::shared_ptr<const Edge>& second) const
{
    return *first < *second;
}

// ======================================================================
// FaceLessThan

bool FaceLessThan::operator () (
    const boost::shared_ptr<const Face>& first,
    const boost::shared_ptr<const Face>& second) const
{
    return *first < *second;
}



// ======================================================================
// VertexLessThanAlong

bool VertexLessThanAlong::operator() (
    const boost::shared_ptr<Vertex>& first,
    const boost::shared_ptr<Vertex>& second) const
{
    return operator() (first->GetVector (), second->GetVector ());
}

double VertexLessThanAlong::operator() (
    const boost::shared_ptr<Vertex>& x) const
{
    return operator() (x->GetVector ());
}


// ======================================================================
// OrientedFaceIndexLessThan

bool OrientedFaceIndexLessThan::operator () (
    const AdjacentOrientedFace& first, const AdjacentOrientedFace& second) const
{
    boost::shared_ptr<OrientedFace> firstOf = first.GetOrientedFace ();
    boost::shared_ptr<OrientedFace> secondOf = second.GetOrientedFace ();
    return firstOf->GetId () <	secondOf->GetId ();
}

bool BodyLessThan (const boost::shared_ptr<Body>& first,
		   const boost::shared_ptr<Body>& second)
{
    return *first < *second;
}

bool BodyLessThanId (const boost::shared_ptr<Body>& first,
		   size_t secondBodyId)
{
    return *first < secondBodyId;
}


// ======================================================================
// BBObjectLessThanAlong

template <typename BBObject>
bool BBObjectLessThanAlong<BBObject>::operator() (
    const BBObject& first, const BBObject& second)
{
    return 
	(m_getAABox (first).*m_corner) ()[m_axis] < 
	(m_getAABox (second).*m_corner) ()[m_axis];
}

template <typename BBObject>
double BBObjectLessThanAlong<BBObject>::operator() (const BBObject& x)
{
    return (m_getAABox (x).*m_corner) ()[m_axis];
}


//======================================================================
// Template instantiations

// =====================================================================
// BBObjectLessThanAlong
template class BBObjectLessThanAlong<Body>;
template class BBObjectLessThanAlong<Foam>;
