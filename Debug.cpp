#include "Debug.h"

void _RuntimeAssert (bool condition,  string& message)
{
    if (! condition)
	throw logic_error (message);
}

