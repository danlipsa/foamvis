/**
 * @file   DerivedData.h
 * @author Dan R. Lipsa
 * @date 4 July 2011
 * @ingroup model
 * @brief Data derived from simulation data such as caches and averages.
 */
#ifndef __DERIVED_DATA_H__
#define __DERIVED_DATA_H__

class AverageCacheT1KDEVelocity;
class ObjectPositions;


/**
 * @brief Data derived from simulation data such as caches and averages.
 */
struct DerivedData
{
    DerivedData (
        boost::shared_ptr<AverageCacheT1KDEVelocity> ac, 
        boost::shared_ptr<ObjectPositions> op) :
        m_averageCache (ac), m_objectPositions (op)
    {
    }
    boost::shared_ptr<AverageCacheT1KDEVelocity> m_averageCache;
    boost::shared_ptr<ObjectPositions> m_objectPositions;
};


#endif //__DERIVED_DATA_H__

// Local Variables:
// mode: c++
// End:
