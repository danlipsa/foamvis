/**
 * @file   Hashes.h
 * @author Dan R. Lipsa
 * @date 26 March 2010
 * Hash functors
 */

#ifndef __HASHES_H__
#define __HASHES_H__

class Face;

struct FaceHash
{
    size_t operator() (const Face& face) const;
    size_t operator () (const Face* f) const
    {
	return operator() (*f);
    }
};


#endif //__HASHES_H__
