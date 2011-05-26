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
    return GetBody ()->GetOrientedFacePtr (GetOrientedFaceIndex ());
}

size_t BodyIndex::GetBodyId () const
{
    return GetBody ()->GetId ();
}


bool BodyIndex::IsOrientedFaceReversed () const
{
    return GetOrientedFace ()->IsReversed ();
}

string BodyIndex::ToString () const
{
    ostringstream ostr;
    ostr << "(body=" << GetBodyId () 
	 << ", ofI=" << GetOrientedFaceIndex () << ")";
    return ostr.str ();
}

