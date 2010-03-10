#include "Debug.h"

void RuntimeAssert (bool condition, const string& message)
{
    if (! condition)
	throw logic_error (message);
}

