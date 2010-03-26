/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "OrientedFace.h"
#include "Debug.h"
#include "DebugStream.h"

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << (of.m_reversed ? "(R) " : "(N) ")
	 << "Oriented Face " << of.GetFace ()-> GetOriginalIndex () 
	 << " " << of.GetFace ()->GetColor () << " "
	 << (of.GetFace ()->IsDuplicate () ? " DUPLICATE" : "")
	 << ": " << endl;
    ostr << of.size () << " edges part of the face:" << endl;
    for (size_t i = 0; i < of.size (); i++)
    {
	const OrientedEdge oe = of.GetOrientedEdge (i);
	ostr << i << ": " << oe << endl;
    }
    ostr << " Face attributes: ";
    return of.m_face->PrintAttributes (ostr);
}

OrientedEdge OrientedFace::GetOrientedEdge (size_t edgeIndex) const
{
    vector<OrientedEdge*>& v = m_face->GetOrientedEdges ();
    RuntimeAssert (edgeIndex < v.size (),
		   "Edge index ", edgeIndex,
		   " greater than the number of edges ", v.size ());
    OrientedEdge* oe;
    if (IsReversed ())
    {
        oe = v[v.size() - 1 - edgeIndex];
    }
    else
    {
        oe = v[edgeIndex];
    }
    bool reversed = oe->IsReversed ();
    if (IsReversed ())
	reversed = ! reversed;
    return OrientedEdge (oe->GetEdge (), reversed);
}

G3D::Vector3 OrientedFace::GetNormal () const
{
    G3D::Vector3 normal = m_face->GetNormal ();
    return m_reversed ? - normal : normal;
}

bool OrientedFace::FitFace (const EdgeFit& edgeFit,
			    OrientedFace::const_iterator* fitPosition,
			    G3D::Vector3* translation) const
{
    for (OrientedFace::const_iterator it = begin (); it != end (); ++it)
    {
	OrientedEdge candidateEdge = *it;
	if (edgeFit.Fits (*this, candidateEdge))
	{
	    *translation = *(edgeFit.m_edge.GetEdge ()->GetBegin ()) - 
		*(candidateEdge.GetEdge ()->GetBegin ());
	    *fitPosition = it;
	    cdbg << "Fitted edge: " << candidateEdge
		 << " translation: " << *translation << " into "
		 << edgeFit << endl;
	    return true;
	}
    }
    return false;
}
