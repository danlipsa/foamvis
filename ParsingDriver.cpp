/**
 * @file   ParsingDriver.h
 * @author Dan R. Lipsa
 *
 * Definitions for the class that drives parsing and scanning
 */
#include "SystemDifferences.h"
#include "ParsingDriver.h"
#include "DebugStream.h"


ParsingDriver::ParsingDriver ()
  : m_debugScanning (false), m_debugParsing (false)
{
}

ParsingDriver::~ParsingDriver ()
{
}

void ParsingDriver::PrintError (
    const EvolverData::location& l, const string& m)
{
	cdbg << l << ": " << m << endl;
}

void ParsingDriver::PrintError (const string& m)
{
  cdbg << m << endl;
}

long ParsingDriver::ReadInteger (char* str, int base)
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

 int ParsingDriver::FIRST_TOKEN = EvolverData::parser::token::PARAMETER;

const char* ParsingDriver::m_keywordTable[] = {
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
    "KEEP_ORIGINALS",
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
    "SPACE_DIMENSION",
    "LENGTH_METHOD_NAME",
    "AREA_METHOD_NAME",
    "QUANTITY",
    "METHOD_INSTANCE",
    "METHOD",
    "SCALAR_INTEGRAND",
    "VIEW_TRANSFORM_GENERATORS",
    "SWAP_COLORS",
    "INTEGRAL_ORDER_1D",
    "ORIGINAL",
    "VOLUME",
    "VOLCONST",
    "LAGRANGE_MULTIPLIER"
};

int ParsingDriver::GetKeywordId (char* keyword)
{
    for (size_t i = 0; 
         i < sizeof (m_keywordTable) / sizeof (m_keywordTable[0]); ++i)
    {
	if (strcasecmp (m_keywordTable[i], keyword) == 0)
	    return i + FIRST_TOKEN;
    }
    return 0;
}

const char* ParsingDriver::GetKeywordString (int id)
{
    
    return m_keywordTable[id - FIRST_TOKEN];
}

int ParsingDriver::Parse (string &f, Data& data)
{
    m_file = f;
    ScanBegin ();
    EvolverData::parser parser (data, m_scanner);
    parser.set_debug_level (m_debugParsing);
    int result = parser.parse ();
    ScanEnd ();
    return result;
}
