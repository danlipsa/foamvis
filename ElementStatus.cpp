/**
 * @file   ElementStatus.h
 * @author Dan R. Lipsa
 * @date 1 May 2010
 *
 * Implementation for ElementStatus
 */

#include "ElementStatus.h"
#include "Debug.h"

ostream& operator<< (ostream& ostr, ElementStatus::Name status)
{
    switch (status)
    {
    case ElementStatus::ORIGINAL:
	ostr << "ORIGINAL";
	break;
    case ElementStatus::DUPLICATE_MADE:
	ostr << "DUPLICATE_MADE";
	break;
    case ElementStatus::DUPLICATE:
	ostr << "DUPLICATE";
	break;
    default:
	RuntimeAssert (false, "Invalid ElementStatus: ", status);
    }
    return ostr;
}
