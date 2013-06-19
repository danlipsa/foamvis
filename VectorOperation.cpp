/**
 * @file   VectorOperation.cpp
 * @author Dan R. Lipsa
 * @date  25 Feb 2013
 *
 * Implementation for VectorOperation
 *
 */

#include "Debug.h"
#include "Enums.h"
#include "VectorOperation.h"

// Private Classes/Functions
// ======================================================================
/*
 * @brief Math operation between two vtkImageData. left = left op right
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
 * @brief Math operation between a vtkImageData and a scalar. left =
 * right op scalar
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


void logValidPoints (vtkSmartPointer<vtkImageData> data)
{
    vtkSmartPointer<vtkCharArray> validPoints = 
	vtkCharArray::SafeDownCast (
            data->GetPointData ()->GetArray (VectorOperation::VALID_NAME));
    if (validPoints != 0)
    {
        vtkIdType tuples = validPoints->GetNumberOfTuples ();
        vtkIdType validTuples = 0;
        for (vtkIdType i = 0; i < tuples; ++i)
        {
            char valid;
            validPoints->GetTupleValue (i, &valid);
            if (valid)
                ++validTuples;
        }
        cdbg << validTuples << " valid of " << tuples << endl;
    }
    else
        cdbg << "all tuples valid" << endl;
}

void convertDataToArrays (
    size_t attribute, vtkSmartPointer<vtkImageData> left, 
    vtkSmartPointer<vtkImageData> right,
    VectorOperation::DataAndValidFlag* l, VectorOperation::DataAndValidFlag* r)
{
    const char* attributeName = BodyAttribute::ToString (attribute);
    *l = VectorOperation::DataAndValidFlag ( 
	vtkFloatArray::SafeDownCast (
	    left->GetPointData ()->GetArray (attributeName)),
	vtkCharArray::SafeDownCast (
            left->GetPointData ()->GetArray (VectorOperation::VALID_NAME)));

    *r = VectorOperation::DataAndValidFlag ( 
	vtkFloatArray::SafeDownCast (
	    right->GetPointData ()->GetArray (attributeName)),
	vtkCharArray::SafeDownCast (
            right->GetPointData ()->GetArray (VectorOperation::VALID_NAME)));
}

bool isValidSetData (size_t i, VectorOperation::DataAndValidFlag left, 
                     VectorOperation::DataAndValidFlag right)
{
    char leftValid, rightValid;
    left.m_valid->GetTupleValue (i, &leftValid);
    right.m_valid->GetTupleValue (i, &rightValid);
    if (! leftValid && ! rightValid)
    {
        char c (0);
        left.m_valid->SetTupleValue (i, &c);
        boost::array<float, BodyAttribute::MAX_NUMBER_OF_COMPONENTS> leftData;
        fill (leftData.begin (), leftData.end (), 0);
        left.m_data->SetTupleValue (i, &leftData[0]);
        return false;
    }
    else
        return true;
}



// VectorOperation
//============================================================================
const char* VectorOperation::VALID_NAME = "vtkValidPointMask";


// VectorOpVector
//============================================================================

void VectorOpVector::operator() (DataAndValidFlag left, DataAndValidFlag right)
{
    size_t components = left.m_data->GetNumberOfComponents ();
    vtkIdType tuples = left.m_data->GetNumberOfTuples ();
    for (vtkIdType i = 0; i < tuples; ++i)
    {
        if (! isValidSetData (i, left, right))
            continue;
        boost::array<float, BodyAttribute::MAX_NUMBER_OF_COMPONENTS> leftData;
        boost::array<float, BodyAttribute::MAX_NUMBER_OF_COMPONENTS> rightData;
        left.m_data->GetTupleValue (i, &leftData[0]);
        right.m_data->GetTupleValue (i, &rightData[0]);
        for (size_t j = 0; j < components; ++j)
        {
            leftData[j] = 
                GetBinaryOperation () (leftData[j], rightData[j]);
        }
        left.m_data->SetTupleValue (i, &leftData[0]);
    }
}


void VectorOpVector::operator () (G3D::Vector3& left, const G3D::Vector3& right)
{
    for (size_t i = 0; i < 3; ++i)
        left[i] = GetBinaryOperation () (left[i], right[i]);
}

// VectorOpScalar
// ===========================================================================

void VectorOpScalar::operator() (
    DataAndValidFlag left, DataAndValidFlag right, double scalar)
{
    size_t components = left.m_data->GetNumberOfComponents ();
    vtkIdType tuples = left.m_data->GetNumberOfTuples ();
    for (vtkIdType i = 0; i < tuples; ++i)
    {
        if (! isValidSetData (i, left, right))
            continue;
        boost::array<float, BodyAttribute::MAX_NUMBER_OF_COMPONENTS> leftData;
        boost::array<float, BodyAttribute::MAX_NUMBER_OF_COMPONENTS> rightData;
        right.m_data->GetTupleValue (i, &rightData[0]);
        for (size_t j = 0; j < components; ++j)
            leftData[j] = GetBinaryOperation () (rightData[j], scalar);
        left.m_data->SetTupleValue (i, &leftData[0]);
    }
}

// standalone functions
// ===========================================================================

void ImageOpImage (
    vtkSmartPointer<vtkImageData> left, 
    vtkSmartPointer<vtkImageData> right, 
    VectorOperation::BinaryOperation f, size_t attribute)
{
    VectorOperation::DataAndValidFlag l, r;
    convertDataToArrays (attribute, left, right, &l, &r);
    VectorOpVector vf (f);
    vf (l, r);
    left->Modified ();
}

void ImageOpScalar (
    vtkSmartPointer<vtkImageData> left, 
    vtkSmartPointer<vtkImageData> right, float scalar,
    VectorOperation::BinaryOperation f, size_t attribute)
{
    VectorOperation::DataAndValidFlag l, r;
    convertDataToArrays (attribute, left, right, &l, &r);
    VectorOpScalar vos(f); 
    vos (l, r, scalar);
    left->Modified ();
}
