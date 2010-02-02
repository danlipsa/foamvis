/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Parses an Evolver DMP file and displays the data from the file.
 */
#include "DataFiles.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "SemanticError.h"
#include "DebugStream.h"

/**
 * \mainpage The Foam Project
 * \section intro_sec Introduction
 *
 * This  project  is  meant  to  visualize and  help  understand  foam
 * modeling data.
 */



/**
 * Functor class used to parse each of the DMP files and store the results
 * in a vector of Data.
 */
class parseFile : public unary_function<QString, void>
{
public:
    /**
     * Constructor
     * @param data Where to store the data parsed from the DMP files
     * @param dir directory where all DMP files are
     */
    parseFile (
	vector<Data*>& data, const QString dir) : 
        m_data (data), m_dir (qPrintable(dir))
    {
    }
    
    /**
     * Parses one file
     * @param f name of the DMP file to be parsed.
     */
    void operator () (QString f)
    {
        int result;
	string file = qPrintable (f);
        Data* data = new Data ();
        m_data.push_back (data);
        ParsingData& parsingData = data->GetParsingData ();
        parsingData.SetDebugParsing (false);
        parsingData.SetDebugScanning (false);
        string fullPath = m_dir + '/' + file;
        cdbg << "Parsing " << file << endl;
        result = parsingData.Parse (fullPath, *data);
        data->ReleaseParsingData ();
        if (result != 0)
        {
            m_data.pop_back ();
            delete data;
        }
    }
private:
    /**
     * Stores the data parsed from the DMP files
     */
    vector<Data*>& m_data;
    /**
     * Directory that stores the DMP files.
     */
    const string m_dir;
};



/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    try
    {
	DataFiles dataFiles;
        if (argc <= 2)
        {
            cdbg << "foam <dir> <filter>\n";
	    cdbg << "where: <dir> is the folder where the data files reside\n"
		 << "       <filter> is filter for the name of "
		"the data files\n"
		 << "       foam reads in Surface Evolver dmp files.\n";
            return 13;
        }
	cdbg << "argv[1]=" << argv[1] << endl;
	cdbg << "argv[2]=" << argv[2] << endl;
	QDir dir (argv[1], argv[2]);
	QStringList files = dir.entryList ();
	for_each (files.begin (), files.end (), 
		  parseFile (dataFiles.GetData (), dir.absolutePath ()));
        if (dataFiles.GetData ().size () != 0)
        {
	    dataFiles.CalculateAABox ();
            QApplication app(argc, argv);
            MainWindow window (dataFiles);
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
