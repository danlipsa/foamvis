/**
 * @file   Hashes.h
 * @author Dan R. Lipsa
 * @date 26 March 2010
 * Hash functors
 */

#include "Hashes.h"
#include "Face.h"

size_t FaceHash::operator() (const Face& face) const
{
    std::size_t seed = 0;
    boost::hash_combine (seed, face.GetOriginalIndex ());
    boost::hash_combine (seed, *face.GetOrientedEdge (0)->GetBegin ());
    return seed;
}
