/**
 * @file   AverageCacheT1KDEVelocity.h
 * @author Dan R. Lipsa
 * @date 10 March 2011
 * @ingroup model
 * @brief Cache of 2D averages for T1KDE and velocity.
 */
#ifndef __AVERAGE_CACHE_T1KDE_VELOCITY_H__
#define __AVERAGE_CACHE_T1KDE_VELOCITY_H__

/**
 * @brief Cache of 2D averages for T1KDE and velocity.
 */
class AverageCacheT1KDEVelocity
{
public:
    void SetT1KDE (vtkSmartPointer<vtkImageData> average)
    {
        m_t1KDE = average;
    }
    vtkSmartPointer<vtkImageData> GetT1KDE () const
    {
        return m_t1KDE;
    }

    void SetVelocity (vtkSmartPointer<vtkImageData> average)
    {
        m_velocityAverage = average;
    }

    vtkSmartPointer<vtkImageData> GetVelocity () const
    {
        return m_velocityAverage;
    }
    
private:
    vtkSmartPointer<vtkImageData> m_t1KDE;
    vtkSmartPointer<vtkImageData> m_velocityAverage;
};


#endif //__AVERAGE_CACHE_T1KDE_VELOCITY_H__

// Local Variables:
// mode: c++
// End:
