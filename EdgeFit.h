/**
 * @file   EdgeFit.h
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Declaration of the EdgeFit class
 */

#ifndef __EDGE_FIT_H__
#define __EDGE_FIT_H__

class OrientedFace;
#include "OrientedEdge.h"

struct EdgeFit
{
public:
    EdgeFit (const OrientedEdge& oe, const G3D::Vector3& n);
    bool Fits (const OrientedFace& face, const OrientedEdge& edge) const;
public:
    friend ostream& operator<< (ostream& ostr, const EdgeFit& ef);

public:
    OrientedEdge m_edge;
    G3D::Vector3 m_normal;
};



#endif //__EDGE_FIT_H__

// Local Variables:
// mode: c++
// End:
