/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Constains the main function of the program.
 */
#include "Data.h"
#include "ParsingData.h"
#include "lexYacc.h"

/**
 * Global object  that stores data read  from a DMP  file (produced by
 * the Surface Evolver software).
 */
Data data;

/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(void)
{
    try
    {
	int parseResult;
	FlexDebugging (0);
	BisonDebugging (0);
	if ((parseResult = yyparse()) == 0)
	{
	    cout << data;
	    cout << data.GetParsingData ();
	}
	return parseResult;
    }
    catch (exception e)
    {
	cerr << "Exception: " << e.what () << endl;
    }
}
