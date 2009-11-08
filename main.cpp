/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Parses an Evolver DMP file and displays the data from the file.
 */
#include <time.h>
#include <QApplication>
#include "Data.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "SemanticError.h"
#include "DebugStream.h"
using namespace std;

class parseFile : public unary_function<QString&, void>
{
public:
    parseFile (vector<Data*>& data, const QString& path) : 
        m_data (data), m_path (path)
    {
    }
    
    void operator () (QString& file)
    {
        int result;
        Data* data = new Data ();
        m_data.push_back (data);
        ParsingData& parsingData = data->GetParsingData ();
        parsingData.SetDebugParsing (false);
        parsingData.SetDebugScanning (false);
        string fullPath = string(qPrintable (m_path)) + 
            '/' + qPrintable (file);
        cdbg << "Parsing " << qPrintable (file) << endl;
        result = parsingData.Parse (fullPath, *data);
        data->ReleaseParsingData ();
        if (result != 0)
        {
            m_data.pop_back ();
            delete data;
        }
    }
private:
    vector<Data*>& m_data;
    const QString& m_path;
};



/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    try
    {
        vector<Data*> data;
        if (argc <= 1)
        {
            cdbg << "foam <dir> <filter>\n";
	    cdbg << "where: <dir> is the folder where the data files reside\n"
		 << "       <filter> is filter for the name of "
		"the data files\n"
		 << "       foam reads in Surface Evolver dmp files.\n";
            return 13;
        }
        else
        {
            QDir dir (argv[1], argv[2]);
            QStringList files = dir.entryList ();
            for_each (files.begin (), files.end (), 
                      parseFile (data, dir.absolutePath ()));
        }
        if (data.size () != 0)
        {
            QApplication app(argc, argv);
            MainWindow window (data);
            window.show();
            return app.exec();
            return 0;
        }
    }
    catch (exception& e)
    {
        cdbg << "Exception: " << e.what () << endl;
    }
}
