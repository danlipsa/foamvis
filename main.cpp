/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Parses an Evolver DMP file and displays the data from the file.
 */
#include "Application.h"
#include "Debug.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "DebugStream.h"

/**
 * \mainpage The Foam Project
 * \section intro_sec Introduction
 *
 * This project visualizes and helps gaining insights into foam
 * simulation data produced by Surface Evolver.
 *
 * Source files and libraries:
 * http://csgalati.swansea.ac.uk/foam/build
 *
 * \verbinclude README.txt
 *
 * \section torus_sec Processing done for the Torus model
 * \subsection onedge_sec Executed when creating an edge (may create duplicate vertices)
 *
 <pre>
The begin vertex (and the middle vertex in quadratic mode) of an edge is 
always defined in the data file (it's not a duplicate).
  if (edge is marked with a *)
    the end vertex is defined in the data file (no DUPLICATE needed)
  else if (edge is marked with a + or -) {
    create a DUPLICATE of the end vertex by translating it
      one domain up for a + and one domain down for a - along each of 
      the three axes. The translation is done relative to the domain where the 
      ORIGINAL end vertex is defined in the data file. This means that
      (* * *) is the domain where the begin vertex is defined in the data file
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
      create a DUPLICATE of currentEdge starting at beginVertex
      set currentEdge to point to the DUPLICATE
    }
    set beginVertex  to be the end vertex of currentEdge
  }
</pre>
 * 
 * \subsection onbody_sec Executed when creating a body (may create duplicate faces)
 *
<pre>
  ...
</pre>


 *
 * \section physical_sec Physical and tesselation edges and vertices
 * In 2D we don't have physical edges. A vertex is "physical" if has
 * >= 3 edges adjacent to it.
 *
 * In 3D, an edge is physical if it has 6 OrientedFaceIndex is part of. (is
 * adjacent with 3 faces)
 * An vertex is physical if it has 4 physical edges adjacent to it.
 *
 *
 * \section Significant space
 * Space is significant for matrices of expressions and for vertex components
 * x, y, z.
 *
 * \section Significant new line

 * and for arrays (2d versus 3d)
 *
 *
 * @todo Use google-breakpad for reporting crashes.  
 *
 * @todo set new line significant only for list of vertices, edges,
 * faces, bodies
 *
 *
 */



/**
 * Functor used to parse a DMP file
 */
class parseFile : public unary_function< QString, boost::shared_ptr<Foam> >
{
public:
    /**
     * Constructor
     * @param data Where to store the data parsed from the DMP files
     * @param dir directory where all DMP files are
     */
    parseFile (QString dir,
	       bool debugParsing = false, bool debugScanning = false) : 
        m_dir (qPrintable(dir)), m_debugParsing (debugParsing),
	m_debugScanning (debugScanning)
    {
    }
    
    /**
     * Parses one file
     * @param f name of the DMP file to be parsed.
     */
    boost::shared_ptr<Foam> operator () (QString f)
    {
	boost::shared_ptr<Foam> foam;
	string file;
	try
	{
	    int result;
	    file = qPrintable (f);
	    ostringstream ostr;
	    ostr << "Parsing " << file << " ..." << endl;
	    cdbg << ostr.str ();
	    foam.reset (new Foam ());
	    foam->GetParsingData ().SetDebugParsing (m_debugParsing);
	    foam->GetParsingData ().SetDebugScanning (m_debugScanning);
	    string fullPath = m_dir + '/' + file;
	    result = foam->GetParsingData ().Parse (fullPath, foam.get ());
	    foam->ReleaseParsingData ();
	    if (result != 0)
		ThrowException ("Error parsing ", fullPath);
	}
	catch (const exception& e)
	{
	    cdbg << "Exception for " << file << ": "
		 << e.what () << endl;
	    foam.reset ();
	}
	return foam;
    }
private:
    /**
     * Directory that stores the DMP files.
     */
    const string m_dir;
    const bool m_debugParsing;
    const bool m_debugScanning;
};


void readOptions (
    int argc, char *argv[],
    bool* debugParsing, bool* debugScanning, bool* textOutput, 
    bool* adjustPressure)
{
    *adjustPressure = true;
    *textOutput = false;
    *debugParsing = false;
    *debugScanning =false;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "opst")) != -1)
	switch (c)
	{
	case 'o':
	    *adjustPressure = false;
	    break;
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
    /**
     * this version number represents the global version from subversion
     * it is generated by:
     * - 'svn commmit/update everyting in the current directory
     * - ./release.pl
     */
    const char* version = "0.4.1728";
    cdbg << "foam version " << version << endl;
    cdbg << "foam [OPTIONS] <dir> <filter>\n";
    cdbg << "foam [OPTIONS] <file> ...\n";
    cdbg << "where: <dir> is the folder where the data files reside\n"
	 << "       <filter> is filter for the name of "
	"the data files\n"
	 << "       <file> is the name of a data file\n"
	 << "OPTIONS:\n"
	 << "       -o : show original pressure values\n"
	 << "       -p : debug parsing\n"
	 << "       -s : debug scanning\n"
	 << "       -t : outputs a text representation of the data\n";
}

QString lastName (const QString& path)
{
    int slashPos = path.lastIndexOf ('/');
    QString ret = path;
    return ret.remove (0, slashPos + 1);
}


void parseFiles (int argc, char *argv[],
		 FoamAlongTime* foamAlongTime,
		 bool debugParsing, bool debugScanning)
{
    QDir dir;
    QStringList files;
    string filePattern;
    QFileInfo fileInfo (argv[optind]);
    if (argc - optind == 0)
    {
	    printHelp ();
	    exit (13);
    }
    if (fileInfo.isDir ())
    {
	if (argc - optind == 2)
	{
	    dir = QDir(argv[optind], argv[optind + 1]);
	    filePattern = string(
		(lastName (dir.absolutePath ()) + '/' + 
		 argv[optind + 1]).toAscii ());
	    files = dir.entryList ();
	}
	else
	{
	    printHelp ();
	    exit (13);
	}
    }
    else
    {
	dir = fileInfo.absoluteDir ();
	for (int i = optind; i < argc; ++i)
	    files << QFileInfo(argv[i]).fileName ();
	filePattern = string (
	    (lastName (dir.absolutePath ()) + 
	     '/' + fileInfo.fileName ()).toAscii ());
    }

    foamAlongTime->SetTimeSteps (files.size ());
    foamAlongTime->SetFilePattern (filePattern);

    QList< boost::shared_ptr<Foam> > foams = QtConcurrent::blockingMapped (
	files,
	parseFile (dir.absolutePath (), debugParsing, debugScanning));
    if (count_if (foams.constBegin (), foams.constEnd (),
		  bl::_1 != boost::shared_ptr<Foam>()) != foams.size ())
	ThrowException ("Could not process all files\n");
    copy (foams.constBegin (), foams.constEnd (),
	  foamAlongTime->GetFoams ().begin ());
}


/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    try
    {
	bool debugParsing, debugScanning, textOutput, adjustPressure;
	FoamAlongTime foamAlongTime;
	readOptions (argc, argv,
		     &debugParsing, &debugScanning, &textOutput, 
		     &adjustPressure);
	parseFiles (argc, argv, &foamAlongTime, debugParsing, debugScanning);
	size_t timeSteps = foamAlongTime.GetTimeSteps ();
        if (timeSteps != 0)
        {
	    foamAlongTime.SetAdjustPressure (adjustPressure);
	    foamAlongTime.Preprocess ();
	    if (textOutput)
		cdbg << foamAlongTime;
	    else
	    {
		int result;
		boost::shared_ptr<Application> app = Application::Get (
		    argc, argv);
		MainWindow window (foamAlongTime);
		window.show();
		result = app->exec();
		app->release ();
		return result;
	    }
            return 0;
        }
	else
	{
	    cdbg << "Error: The patern provided does not match any file" 
		 << endl;
	}
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
    }
}
