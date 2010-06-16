/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 24 May 2010
 *
 * Implementation of the BodyIndex class
 */

#include "BodyIndex.h"
#include "Body.h"
#include "OrientedFace.h"

boost::shared_ptr<OrientedFace>  BodyIndex::GetOrientedFace () const
{
    return GetBody ()->GetOrientedFace (GetOrientedFaceIndex ());
}

size_t BodyIndex::GetBodyId () const
{
    return GetBody ()->GetId ();
}


bool BodyIndex::IsOrientedFaceReversed () const
{
    return GetOrientedFace ()->IsReversed ();
}

// Static and friends methods
// ======================================================================

ostream& operator<< (ostream& ostr, const BodyIndex& bi)
{
    return ostr << "(body=" << bi.GetBodyId () 
		<< ", ofI=" << bi.GetOrientedFaceIndex () << ")";
}
