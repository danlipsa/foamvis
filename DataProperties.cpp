#include "DataProperties.h"
#include "Debug.h"

void DataProperties::SetDimension (size_t dimension) 
{
    RuntimeAssert (dimension == 2 || dimension == 3,
                   "Invalid space dimension: ", dimension);
    m_dimension = Dimension::Enum (dimension);
}

