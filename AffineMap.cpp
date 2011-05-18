/**
 * @file   AffineMap.cpp
 * @author Dan R. Lipsa
 * @date 18 May 2011
 *
 * Implementation of the AffineMap class
 */

#include "AffineMap.h"
#include "Debug.h"

void AffineMap::Set (size_t i, double value)
{
    if (i < 2)
	m_translation[i] = value;
    else if (i == 2)
	m_angle = value;
    else
	ThrowException ("AffineMap: invalid index", i);
}
