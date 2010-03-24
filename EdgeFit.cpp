/**
 * @file   EdgeFit.cpp
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Definition of the EdgeFit class
 */

#include "EdgeFit.h"
#include "OrientedFace.h"
#include "DebugStream.h"

ostream& operator<< (ostream& ostr, const EdgeFit& ef)
{
    ostr << "EdgeFit: oriented edge: " << ef.m_edge
	 << " normal: " << ef.m_normal << endl;
    return ostr;
}

bool EdgeFit::Fits (const OrientedFace& face, const OrientedEdge& edge) const
{
    G3D::Vector3 normal = face.GetNormal ();
    bool edgeFits = m_edge.Fits (edge );
    bool orientationFits = /*(normal.dot (m_normal) >= 0)*/true;
    if (edgeFits && ! orientationFits)
	cdbg << "Edge fits but orientation does not:" << endl
	     << face << edge << " angle: " 
	     << acos (normal.dot (m_normal)) * 180 / M_PI << " degrees"
	     << endl;
    return edgeFits && orientationFits;
}

