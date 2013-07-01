/**
 * @file   VectorOperation.h
 * @author Dan R. Lipsa
 * @date  25 Feb. 2013
 * @ingroup average
 *
 * @brief Math operations for vtkImageData, used for 3D average computation.
 */

#ifndef __VECTOR_OPERATION_H__
#define __VECTOR_OPERATION_H__

/**
 * @brief Math operation for vtkImageData
 */
class VectorOperation
{
public:
    typedef boost::function<double (double, double)> BinaryOperation;
    /**
     * @brief A float array for data and bool array to specify which
     *        data points are valid.
     */
    struct DataAndValidFlag
    {
        DataAndValidFlag ()
        {
        }
        DataAndValidFlag (vtkSmartPointer<vtkFloatArray> data,
                          vtkSmartPointer<vtkCharArray> valid) :
            m_data (data), m_valid (valid)
        {
        }

        vtkSmartPointer<vtkFloatArray> m_data;
        vtkSmartPointer<vtkCharArray> m_valid;
    };
public:
    VectorOperation (BinaryOperation f) : 
        m_f (f)
    {
    }

    BinaryOperation GetBinaryOperation () const
    {
        return m_f;
    }

public:
    static const char* VALID_NAME;

private:
    BinaryOperation m_f;
};

void ImageOpImage (
    vtkSmartPointer<vtkImageData> left, 
    vtkSmartPointer<vtkImageData> right, 
    VectorOperation::BinaryOperation f, size_t attribute);
void ImageOpScalar (
    vtkSmartPointer<vtkImageData> left, 
    vtkSmartPointer<vtkImageData> right, float scalar,
    VectorOperation::BinaryOperation f, size_t attribute);



#endif //__VECTOR_OPERATION_H__

// Local Variables:
// mode: c++
// End:
