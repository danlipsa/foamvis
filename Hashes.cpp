/**
 * @file   Hashes.h
 * @author Dan R. Lipsa
 * @date 26 March 2010
 * Hash functors
 */

#include "Face.h"
#include "Hashes.h"
#include "OrientedEdge.h"
#include "Vertex.h"

size_t FaceHash::operator() (const Face& face) const
{
    std::size_t seed = 0;
    boost::hash_combine (seed, face.GetId ());
    boost::hash_combine (seed, *face.GetOrientedEdge (0)->GetBegin ());
    return seed;
}
