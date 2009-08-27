/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Constains the main function of the program.
 */
#include <QApplication>
#include "Data.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "SemanticError.h"
#include "DebugStream.h"
using namespace std;

/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    try
    {
        Data data;
        int result;
        if (argc <= 1)
        {
            cdbg << "foam <input_file>\n";
            return 13;
        }
        else
        {
            ParsingData& parsingData = data.GetParsingData ();
            parsingData.SetDebugParsing (true);
            parsingData.SetDebugScanning (true);
            result = parsingData.Parse (argv[1], data);
        }
        if (result == 0)
        {
            QApplication app(argc, argv);
            MainWindow window (&data);
            window.show();
            return app.exec();
            return 0;
        }
        else
            return result;
    }
	catch (exception& e)
    {
        cdbg << "Exception: " << e.what () << endl;
    }
}
