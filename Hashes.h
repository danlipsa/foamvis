/**
 * @file   Hashes.h
 * @author Dan R. Lipsa
 * @date 26 March 2010
 * @brief Hash functors
 * @ingroup utils
 */

#ifndef __HASHES_H__
#define __HASHES_H__

/**
 * @brief Hash computation for a Vector3int16
 */
struct Vector3int16Hash
{
    std::size_t operator() (const G3D::Vector3int16& v) const
    {
	std::size_t seed = 0;
	boost::hash_combine (seed, v.x);
	boost::hash_combine (seed, v.y);
	boost::hash_combine (seed, v.z);
	return seed;
    }
};

/**
 * @brief Hash computation for a Vector3
 */
struct Vector3Hash
{
    size_t operator() (const G3D::Vector3& v) const
    {
	std::size_t seed = 0;
	boost::hash_combine (seed, v.x);
	boost::hash_combine (seed, v.y);
	boost::hash_combine (seed, v.z);
	return seed;
    }
};


#endif //__HASHES_H__
