/**
 * @file   EdgeFit.cpp
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Definition of the EdgeFit class
 */

#include "EdgeFit.h"
#include "OrientedFace.h"

ostream& operator<< (ostream& ostr, const EdgeFit& ef)
{
    ostr << "EdgeFit: oriented edge: " << ef.m_edge
	 << " normal: " << ef.m_normal << endl;
    return ostr;
}

bool EdgeFit::Fits (const OrientedFace& face) const
{
    G3D::Vector3 edgeDirection = *m_edge.GetEnd () - *m_edge.GetBegin ();
    G3D::Vector3 crossNormals = face.GetNormal ().cross (m_normal);
    return edgeDirection.dot (crossNormals) > 0;
}

