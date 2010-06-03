#include "Debug.h"

void RuntimeAssert (bool condition, const string& message)
{
    if (! condition)
	throw logic_error (message);
}


void ThrowException (const string& message)
{
    RuntimeAssert (false, message);
}
