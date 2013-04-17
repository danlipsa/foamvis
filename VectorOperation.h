/**
 * @file   VectorOperation.h
 * @author Dan R. Lipsa
 * @date  25 Feb. 2013
 *
 * Math operations for vtkImageData
 */

#ifndef __VECTOR_OPERATION_H__
#define __VECTOR_OPERATION_H__


class VectorOperation
{
public:
    typedef boost::function<double (double, double)> BinaryOperation;
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



/*
 * left = left op right
 */
class VectorOpVector : public VectorOperation
{
public:
    VectorOpVector (BinaryOperation f) : 
	VectorOperation (f)
    {
    }
    
    void operator() (DataAndValidFlag left,DataAndValidFlag right);
    void operator () (G3D::Vector3& left, const G3D::Vector3& right);
};

/**
 * left = right op scalar
 */
class VectorOpScalar : public VectorOperation
{
public:
    VectorOpScalar (BinaryOperation f) : 
        VectorOperation (f)
    {
    }
    
    void operator() (
        DataAndValidFlag left, DataAndValidFlag right, double scalar);
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
