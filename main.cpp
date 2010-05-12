/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Parses an Evolver DMP file and displays the data from the file.
 */
#include "Data.h"
#include "DataFiles.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "DebugStream.h"

/**
 * \mainpage The Foam Project
 * \section intro_sec Introduction
 *
 * This project visualizes and helps understand foam modeling data produced
 * by Surface Evolver.
 *
 * \section torus_sec Processing done for the Torus model
 * \subsection onedge_sec Executed when creating an edge (may create duplicate vertices)
 *
 <pre>
The first vertex of an edge is always defined in the data file (it's not a 
duplicate).
  if (edge is marked with a *)
    the second vertex is defined in the data file (no DUPLICATE needed)
  else if (edge is marked with a + or -) {
    create a DUPLICATE of the second vertex by translating it
      one domain up for a + and one domain down for a - along each of 
      the three axes. The translation is done relative to the domain where the 
      ORIGINAL second vertex is defined in the data file. This means that
      (* * *) is the domain where the second vertex is defined in the data file
      NOT the original domain.
  }
 </pre>
 * 
 * \subsection onface_sec Executed when creating a face (may create duplicate edges)
 *
<pre>
  first vertex of the face is defined in the data file (not a DUPLICATE).
  set beginVertex to be the first vertex of the face
  foreach (currentEdge, edges in the face) {
    if (the beginVertex does not match the begin vertex of the currentEdge) {
      create a DUPLICATE edge
      set beginVertex  to be the end vertex of the potential DUPLICATE of
        currentEdge
    }
  }
</pre>
*/



/**
 * Functor used to parse each of the DMP files and store the results
 * in a vector of Data.
 */
class parseFile : public unary_function<QString, bool>
{
public:
    /**
     * Constructor
     * @param data Where to store the data parsed from the DMP files
     * @param dir directory where all DMP files are
     */
    parseFile (vector<Data*>& data, QString dir,
	       bool debugParsing = false, bool debugScanning = false) : 
        m_data (data), m_dir (qPrintable(dir)), m_debugParsing (debugParsing),
	m_debugScanning (debugScanning)
    {
    }
    
    /**
     * Parses one file
     * @param f name of the DMP file to be parsed.
     */
    bool operator () (QString f)
    {
        int result;
	string file = qPrintable (f);
	cdbg << "Parsing " << file << " ..." << endl;
        Data* data = new Data ();
        m_data.push_back (data);
        ParsingData& parsingData = data->GetParsingData ();
        parsingData.SetDebugParsing (m_debugParsing);
        parsingData.SetDebugScanning (m_debugScanning);
        string fullPath = m_dir + '/' + file;
        result = parsingData.Parse (fullPath, *data);
        data->ReleaseParsingData ();
        if (result != 0)
        {
            m_data.pop_back ();
            delete data;
	    return false;
        }
	else
	    return true;
    }
private:
    /**
     * Stores the data parsed from the DMP files
     */
    vector<Data*>& m_data;
    /**
     * Directory that stores the DMP files.
     */
    string m_dir;
    bool m_debugParsing;
    bool m_debugScanning;
};


void readOptions (
    int argc, char *argv[],
    bool* debugParsing, bool* debugScanning, bool* textOutput)
{
    *textOutput = false;
    *debugParsing = false;
    *debugScanning =false;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "pst")) != -1)
	switch (c)
	{
	case 'p':
	    *debugParsing = true;
	    break;
	case 's':
	    *debugScanning = true;
	    break;
	case 't':
	    *textOutput = true;
	    break;
	case '?':
	    if (isprint (optopt))
		cdbg << "Unknown option `-" << optopt << "'." << endl;
	    else
		cdbg << "Unknown option character `x" 
		     << hex << optopt << "'."  << endl;
	    exit (13);
	default:
	    exit (13);
	}
}

void printHelp ()
{
    cdbg << "foam [OPTIONS] <dir> <filter>\n";
    cdbg << "foam [OPTIONS] <file>\n";
    cdbg << "where: <dir> is the folder where the data files reside\n"
	 << "       <filter> is filter for the name of "
	"the data files\n"
	 << "       <file> is the name of a data file\n"
	 << "OPTIONS:\n"
	 << "       -p : debug parsing\n"
	 << "       -s : debug scanning\n"
	 << "       -t : outputs a text representation of the data\n";
}

void parseFiles (int argc, char *argv[],
		 DataFiles* dataFiles,
		 bool debugParsing, bool debugScanning)
{
    switch (argc - optind)
    {
    case 1:
    {
	QFileInfo fileInfo (argv[optind]);
	QDir dir = fileInfo.absoluteDir ();
	if (! parseFile (dataFiles->GetData (), dir.absolutePath (),
			 debugParsing, debugScanning) (
			     fileInfo.fileName ()))
	    exit (13);
	break;
    }
    case 2:
    {
	QDir dir (argv[optind], argv[optind + 1]);
	QStringList files = dir.entryList ();
	if (count_if (
		files.begin (), files.end (), 
		parseFile (dataFiles->GetData (), dir.absolutePath (), 
			   debugParsing, debugScanning))
	    != files.size ())
	    exit (13);
	break;
    }
    default:
	printHelp ();
	exit (13);
    }
}


class Application: public QApplication
{
public:
    Application(int &c, char **v): QApplication(c, v) {}
    virtual bool notify(QObject *rec, QEvent *ev)
    {
	try
	{
	    return QApplication::notify(rec, ev);
	}
	catch (exception& e)
	{
	    cdbg << "Exception: " << e.what () << endl;
	    return false;
	}
    }
};

/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    try
    {
	bool debugParsing, debugScanning, textOutput;
	DataFiles dataFiles;
	readOptions (argc, argv,
		     &debugParsing, &debugScanning, &textOutput);
	parseFiles (argc, argv, &dataFiles,
		    debugParsing, debugScanning);
        if (dataFiles.GetData ().size () != 0)
        {
	    dataFiles.CalculateAABox ();
	    if (textOutput)
		cdbg << dataFiles;
	    else
	    {
		Application app(argc, argv);
		MainWindow window (dataFiles);
		window.show();
		return app.exec();
	    }
            return 0;
        }
	else
	{
	    cdbg << "Error: The patern provided does not match any file" 
		 << endl;
	}
    }
    catch (exception& e)
    {
        cdbg << "Exception: " << e.what () << endl;
    }
}
