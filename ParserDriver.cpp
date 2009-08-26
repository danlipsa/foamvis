#include "SystemDifferences.h"
#include "ParserDriver.h"
#include "DebugStream.h"
using namespace std;

ParserDriver::ParserDriver ()
  : m_debugScanning (false), m_debugParsing (false)
{
}

ParserDriver::~ParserDriver ()
{
}

void ParserDriver::PrintError (
    const EvolverData::location& l, const string& m)
{
	cdbg << l << ": " << m << endl;
}

void ParserDriver::PrintError (const string& m)
{
  cdbg << m << endl;
}

long ParserDriver::ReadInteger (char* str, int base)
{
    char *tail = str;
    errno = 0;
    long i = strtol (str, &tail, base);
    if (errno)
    {
		PrintError (string("Scanner: long overflow ") + str);
	exit(13);
    }
    return i;
}

const int ParserDriver::FIRST_TOKEN = EvolverData::parser::token::PARAMETER;

const char* ParserDriver::m_keywordTable[] = {
    "PARAMETER",
    "PERIODS",
    "VIEW_MATRIX",
    "VERTICES",
    "EDGES",
    "FACES",
    "BODIES",
    "READ",
// extra attributes
    "DEFINE",
    "ATTRIBUTE",
    "VERTEX",
    "EDGE",
    "FACET",
    "BODY",
    "INTEGER",
    "REAL",
// colors
    "COLOR",
    "BLACK",
    "BLUE",
    "GREEN",
    "CYAN",
    "RED",
    "MAGENTA",
    "BROWN",
    "LIGHTGRAY",
    "DARKGRAY",
    "LIGHTBLUE",
    "LIGHTGREEN",
    "LIGHTCYAN",
    "LIGHTRED",
    "LIGHTMAGENTA",
    "YELLOW",
    "WHITE",
    "CLEAR",
// additional parameters,
    "STRING",
    "SOAPFILM",
    "LINEAR",
    "QUADRATIC",
    "SIMPLEX_REPRESENTATION",
    "TOTAL_TIME",
    "CONSTRAINT_TOLERANCE",
    "SYMMETRIC_CONTENT",
    "SCALE",
    "NOWALL",
    "GLOBAL",
    "CONTENT",
    "C1",
    "C2",
    "C3",
    "CONVEX",
    "ENERGY",
    "E1",
    "E2",
    "E3",
    "FUNCTION",
    "NONNEGATIVE",
    "NONPOSITIVE",
    "FORMULA",
    "CONSTRAINT",
    "EQUATION",
    "TORUS_FILLED",
    "TORUS",
    "ORIGINAL",
    "VOLUME",
    "LAGRANGE_MULTIPLIER"
};

int ParserDriver::GetKeywordId (const char* keyword)
{
    for (int i = 0; 
         i < sizeof (m_keywordTable) / sizeof (m_keywordTable[0]);
         i++)
    {
	if (strcasecmp (m_keywordTable[i], 
                         keyword) == 0)
	    return i + FIRST_TOKEN;
    }
    return 0;
}

const char* ParserDriver::GetKeywordString (int id)
{
    
    return m_keywordTable[id - FIRST_TOKEN];
}
