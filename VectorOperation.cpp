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

// VectorOperation
//============================================================================
const char* VectorOperation::VALID_NAME = "vtkValidPointMask";


// VectorOpVector
//============================================================================

void VectorOpVector::operator() (ValidData left, ValidData right)
{
    size_t components = left.m_data->GetNumberOfComponents ();
    vtkIdType tuples = left.m_data->GetNumberOfTuples ();
    for (vtkIdType i = 0; i < tuples; ++i)
    {
        float leftData[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
        float rightData[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
        char leftValid, rightValid;
        left.m_valid->GetTupleValue (i, &leftValid);
        right.m_valid->GetTupleValue (i, &rightValid);
        if (! leftValid || ! rightValid)
        {
            char c (0);
            left.m_valid->SetTupleValue (i, &c);
            continue;
        }
        left.m_data->GetTupleValue (i, leftData);
        right.m_data->GetTupleValue (i, rightData);
        for (size_t j = 0; j < components; ++j)
        {
            leftData[j] = 
                GetBinaryOperation () (leftData[j], rightData[j]);
        }
        left.m_data->SetTupleValue (i, leftData);
    }
}


void VectorOpVector::operator () (G3D::Vector3& left, const G3D::Vector3& right)
{
    for (size_t i = 0; i < 3; ++i)
        left[i] = GetBinaryOperation () (left[i], right[i]);
}

// VectorOpScalar
// ===========================================================================

void VectorOpScalar::operator() (ValidData left, ValidData right, double scalar)
{
    size_t components = left.m_data->GetNumberOfComponents ();
    vtkIdType tuples = left.m_data->GetNumberOfTuples ();
    for (vtkIdType i = 0; i < tuples; ++i)
    {
        float leftData[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
        float rightData[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
        char leftValid, rightValid;
        left.m_valid->GetTupleValue (i, &leftValid);
        right.m_valid->GetTupleValue (i, &rightValid);
        if (! leftValid || ! rightValid)
        {
            char c (0);
            left.m_valid->SetTupleValue (i, &c);
            continue;
        }
        right.m_data->GetTupleValue (i, rightData);
        for (size_t j = 0; j < components; ++j)
        {
            leftData[j] = GetBinaryOperation () (rightData[j], scalar);
        }
        left.m_data->SetTupleValue (i, leftData);
    }
}

// standalone functions
// ===========================================================================
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
    VectorOperation::ValidData* l, VectorOperation::ValidData* r)
{
    const char* attributeName = BodyAttribute::ToString (attribute);
    *l = VectorOperation::ValidData ( 
	vtkFloatArray::SafeDownCast (
	    left->GetPointData ()->GetArray (attributeName)),
	vtkCharArray::SafeDownCast (
            left->GetPointData ()->GetArray (VectorOperation::VALID_NAME)));

    *r = VectorOperation::ValidData ( 
	vtkFloatArray::SafeDownCast (
	    right->GetPointData ()->GetArray (attributeName)),
	vtkCharArray::SafeDownCast (
            right->GetPointData ()->GetArray (VectorOperation::VALID_NAME)));
}

void ImageOpImage (
    vtkSmartPointer<vtkImageData> left, 
    vtkSmartPointer<vtkImageData> right, 
    VectorOperation::BinaryOperation f, size_t attribute)
{
    VectorOperation::ValidData l, r;
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
    VectorOperation::ValidData l, r;
    convertDataToArrays (attribute, left, right, &l, &r);
    VectorOpScalar vos(f); 
    vos (l, r, scalar);
    left->Modified ();
}
