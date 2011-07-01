/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 24 May 2010
 *
 * Implementation of the AdjacentBody class
 */

#include "AdjacentBody.h"
#include "Body.h"
#include "OrientedFace.h"

boost::shared_ptr<OrientedFace>  AdjacentBody::GetOrientedFace () const
{
    return GetBody ()->GetOrientedFacePtr (GetOrientedFaceIndex ());
}

size_t AdjacentBody::GetBodyId () const
{
    return GetBody ()->GetId ();
}


bool AdjacentBody::IsOrientedFaceReversed () const
{
    return GetOrientedFace ()->IsReversed ();
}

string AdjacentBody::ToString () const
{
    ostringstream ostr;
    ostr << "(body=" << GetBodyId () 
	 << ", ofI=" << GetOrientedFaceIndex () << ")";
    return ostr.str ();
}

