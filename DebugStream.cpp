/**
 * @file   DebugStream.h
 * @author Dan R. Lipsa
 *
 * Stream for printing  debug messages. All debug messages  have to go
 * to cdbg.  It works with both MS Visual C++ and with GCC
 */

#include "DebugStream.h"

#ifdef _MSC_VER

DebugStream cdbg;

#else //_MSC_VER

ostream& cdbg = cerr;

#endif //_MSC_VER


MeasureTime::MeasureTime ()
{
    m_start = clock ();
}

void MeasureTime::EndInterval (const char* intervalName)
{
    clock_t end = clock ();
    cdbg << intervalName << ": " 
	 << (end - m_start) * 1000 / CLOCKS_PER_SEC << " ms" << endl;
    m_start = end;
}
