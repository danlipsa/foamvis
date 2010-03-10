/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "OrientedFace.h"
#include "Debug.h"

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << (of.m_reversed ? "(R)" : "(N)");
    if (of.m_reversed)
        of.m_face->ReversePrint (ostr);
    else
        ostr << *of.m_face;
    return ostr;
}

Vertex* OrientedFace::getBegin (size_t edgeIndex) const
{
    OrientedEdge* edge = getOrientedEdge (edgeIndex);
    if (IsReversed ())
        return edge->GetEnd ();
    else
        return edge->GetBegin ();
}

Vertex* OrientedFace::getEnd (size_t edgeIndex) const
{
    OrientedEdge* edge = getOrientedEdge (edgeIndex);
    if (IsReversed ())
        return edge->GetBegin ();
    else
        return edge->GetEnd ();
}

OrientedEdge* OrientedFace::getOrientedEdge (size_t edgeIndex) const
{
    vector<OrientedEdge*>& v = m_face->GetOrientedEdges ();
    RuntimeAssert (edgeIndex < v.size (),
		   "Edge index ", edgeIndex,
		   " greater than the number of edges ", v.size ());
    OrientedEdge* edge;
    if (IsReversed ())
    {
        edge = v[v.size() - 1 - edgeIndex];
    }
    else
    {
        edge = v[edgeIndex];
    }
    return edge;
}
