/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "OrientedFace.h"
using namespace std;

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << (of.m_reversed ? "(R)" : "(N)");
    if (of.m_reversed)
        of.m_face->ReversePrint (ostr);
    else
        ostr << *of.m_face;
    return ostr;
}

const Vertex* OrientedFace::GetBegin (unsigned int edgeIndex) const
{
    const vector<OrientedEdge*>& v = m_face->GetOrientedEdges ();
    if (edgeIndex >= v.size ())
    {
        ostringstream ostr;
        ostr << "Edge index " << edgeIndex 
             << " greater than the number of edges " << v.size () << ends;
        throw invalid_argument (ostr.str ());
    }
    OrientedEdge* edge;
    if (IsReversed ())
    {
        edge = v[v.size() - 1 - edgeIndex];
        return edge->GetEnd ();
    }
    else
    {
        edge = v[edgeIndex];
        return edge->GetBegin ();
    }
}

const Vertex* OrientedFace::GetEnd (unsigned int edgeIndex) const
{
    const vector<OrientedEdge*>& v = m_face->GetOrientedEdges ();
    if (edgeIndex >= v.size ())
    {
        ostringstream ostr;
        ostr << "Edge index " << edgeIndex 
             << " greater than the number of edges " << v.size () << ends;
        throw invalid_argument (ostr.str ());
    }
    OrientedEdge* edge;
    if (IsReversed ())
    {
        edge = v[v.size() - 1 - edgeIndex];
        return edge->GetBegin ();
    }
    else
    {
        edge = v[edgeIndex];
        return edge->GetEnd ();
    }
}
