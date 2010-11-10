/**
 * @file   ParsingDriver.h
 * @author Dan R. Lipsa
 *
 * Definitions for the class that drives parsing and scanning
 */
#include "SystemDifferences.h"
#include "ParsingDriver.h"
#include "DebugStream.h"
#include "Debug.h"


ParsingDriver::ParsingDriver ()
  : m_debugScanning (false), m_debugParsing (false)
{
}

ParsingDriver::~ParsingDriver ()
{
}

void ParsingDriver::PrintError (
    const EvolverData::location& l, const char* message)
{
	cdbg << l << ": " << message << endl;
}

long ParsingDriver::ReadInteger (char* str, int base)
{
    char *tail = str;
    errno = 0;
    long i = strtol (str, &tail, base);
    if (errno)
	ThrowException (string() + "Scanner: long overflow " + str);
    return i;
}

 int const ParsingDriver::FIRST_TOKEN = EvolverData::parser::token::PARAMETER;

const char* ParsingDriver::KEYWORD_TABLE[] = {
    "PARAMETER",
    "PERIODS",
    "DISPLAY_PERIODS",
    "DISPLAY_ORIGIN",
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
    "TEMPERATURE",
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
    "INFO_ONLY",
    "CONSERVED",
    "E1",
    "E2",
    "E3",
    "FUNCTION",
    "PROCEDURE",
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
    "FIXED",
    "NO_REFINE",
    "SCALAR_INTEGRAND",
    "VIEW_TRANSFORM_GENERATORS",
    "SWAP_COLORS",
    "INTEGRAL_ORDER_1D",
    "ORIGINAL",
    "VOLUME",
    "VOLCONST",
    "ACTUAL_VOLUME",
    "LAGRANGE_MULTIPLIER",
    "PRESSURE",
    "CONSTRAINTS",
    "DENSITY",
    "TENSION",
    "CLIP_COEFF",
    "AREA_NORMALIZATION",
    "MODULUS",
    "SUPPRESS_WARNING"
};

int ParsingDriver::GetKeywordId (char* keyword)
{
    for (size_t i = 0; 
         i < sizeof (KEYWORD_TABLE) / sizeof (KEYWORD_TABLE[0]); ++i)
    {
	if (strcasecmp (KEYWORD_TABLE[i], keyword) == 0)
	    return i + FIRST_TOKEN;
    }
    return 0;
}

const char* ParsingDriver::GetKeywordString (int id)
{
    
    return KEYWORD_TABLE[id - FIRST_TOKEN];
}

int ParsingDriver::Parse (string &f, Foam& data)
{
    m_file = f;
    ScanBegin ();
    EvolverData::parser parser (data, m_scanner);
    parser.set_debug_level (m_debugParsing);
    int result = parser.parse ();
    ScanEnd ();
    return result;
}
