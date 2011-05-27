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
 * The folowing section lists the content of README.txt
 * \verbinclude README.txt
 *
 * \section sec_torus Processing done for the Torus model
 * \subsection sec_onedge Executed when creating an edge (may create duplicate vertices)
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
 * \subsection sec_onface Executed when creating a face (may create duplicate edges)
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
 * \subsection sec_onbody Executed when creating a body (may create duplicate faces)
 *
<pre>
  ...
</pre>
 *
 * \section sec_physical_tesselation Physical and tesselation edges and vertices
 * In 2D we don't have physical edges. A vertex is "physical" if has
 * >= 3 edges adjacent to it.
 *
 * In 3D, an edge is physical if it has 6 OrientedFaceIndex is part of. (is
 * adjacent with 3 faces)
 * An vertex is physical if it has 4 physical edges adjacent to it.
 *
 *
 * \section sec_space Significant space
 * For matrices of expressions and for vertex components
 * x, y, z.
 *
 * \section sec_new_line Significant new line
 * For arrays (2d versus 3d)
 *
 * \section sec_t1s Format of additional text file containing T1s
 * A line that starts with a # is a comment line \n
 * Each line contains three entries separated by space: time_step, x, y
 * where x and y are the coordinates of the T1 in object space \n
 * The first time step is 1. \n
 * A T1 labeled with timestep T occurs between T and T+1.\n
 *
 * @todo Why time displacement does not go all the way to max Z?
 * @todo Use vertex arrays for center paths
 * @todo Use google-breakpad for reporting crashes.  
 *
 * @todo set new line significant only for list of vertices, edges,
 * faces, bodies
 *
 *
 */


const char* optionName[] =
{
    "constraint-rotation",
    "debug-parsing",
    "debug-scanning",
    "dmp-file",
    "help",
    "original-pressure",
    "output-text",
    "t1s",
    "use-original",
    "version"
};


struct Option
{
    enum Enum
    {
	CONSTRAINT_ROTATION,    // a
	DEBUG_PARSING,          // p
	DEBUG_SCANNING,         // s
	DMP_FILE,
	HELP,
	ORIGINAL_PRESSURE,      // o
	OUTPUT_TEXT,            // t
	T1S,                    // r
	USE_ORIGINAL,
	VERSION,
	COUNT
    };
};


void validate(boost::any& v, const std::vector<std::string>& values,
              ConstraintRotationNames* ignore1, int ignore2)
{
    (void) ignore1;(void)ignore2;
    ConstraintRotationNames am;
    boost::tokenizer<> tok (values[0]);
    istringstream istr;
    boost::tokenizer<>::iterator it = tok.begin ();
    if (it == tok.end ())
	goto error;
    istr.str (*it);
    istr >> am.m_constraintIndex;
    --am.m_constraintIndex;
    if (++it == tok.end ())
	goto error;
    am.m_xName = *it;
    if (++it == tok.end ())
	goto error;
    am.m_yName = *it;
    if (++it == tok.end ())
	goto error;
    am.m_angleName = *it;
    if (++it != tok.end ())
	goto error;
    v = boost::any(am);
    return;

error:
    throw invalid_argument (
	"--constraint-rotation needs four parameters: "
	"\"<xName> <yName> <angleName> <constraint>\"");
}


void parseOptions (int argc, char *argv[],
		   string* t1sFile,
		   vector<string>* fileNames,
		   ConstraintRotationNames* constraintRotationNames,
		   po::variables_map* vm)
{
    // Declare the supported options.
    po::options_description genericOptions(
	"foam [OPTIONS] <files> ...\n"
	"<files> - one or more DMP files\n"
	"OPTIONS");
    genericOptions.add_options()
	(optionName[Option::CONSTRAINT_ROTATION], 
	 po::value<ConstraintRotationNames>(constraintRotationNames), 
	 "read a rotation for a constraint.\n"
	 "arg=\"<constraint> <x> <y> <angle>\" where " 
	 "<constraint> specifies the constraint number, <x>, <y> specify "
	 "names for parameters that store the center of rotation and "
	 "<angle> specifies the name of the parameter that stores "
	 "the rotation angle.")
	(optionName[Option::DEBUG_PARSING], "debug parsing")	    
	(optionName[Option::DEBUG_SCANNING], "debug scanning")
	(optionName[Option::HELP], "produce help message")
	(optionName[Option::ORIGINAL_PRESSURE],
	 "show original pressure values")	    
	(optionName[Option::OUTPUT_TEXT],
	 "outputs a text representation of the data")	    
	(optionName[Option::T1S],
	 po::value<string>(t1sFile), 
	 "read T1 positions.\n"
	 "arg=<file> where <file> specifies a text file with "
	 "T1 times and positions. Reading T1s won't work if you "
	 "skip time steps")
	(optionName[Option::USE_ORIGINAL], "use ORIGINAL atrribute "
	 "to figure out the body id.")
	(optionName[Option::VERSION], "print version information")
	;
    po::options_description hiddenOptions("Hidden options");
    hiddenOptions.add_options()
	("dmp-file", po::value< vector<string> >(fileNames), "dmp file")
	;
    po::options_description options;
    options.add(genericOptions).add(hiddenOptions);

    po::positional_options_description positionalOptions;
    positionalOptions.add(optionName[Option::DMP_FILE], -1);

    po::store(po::command_line_parser (argc, argv).
	      options (options).positional (positionalOptions).
	      run (), *vm);
    po::notify(*vm);
    if (vm->count (optionName[Option::HELP])) {
	cout << genericOptions << "\n";
	exit (0);
    }
    if (vm->count (optionName[Option::VERSION]))
    {
	/**
	 * DO NOT CHANGE
	 * this version number it is generated by release.pl
	 */
	const char* version = "0.5.1928";
	cout << "foam version " << version;
#ifdef QT_NO_DEBUG
	cout << " RELEASE";
#else
	cout << " DEBUG";
#endif
	cout << endl;
	exit (0);
    }
    if (argc == 1 || ! vm->count (optionName[Option::DMP_FILE]))
    {
	cerr << "No DMP file specified" << endl;
	cerr << genericOptions << endl;
	exit (13);
    }
}



/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    try
    {
	FoamAlongTime foamAlongTime;
	string t1sFile;
	vector<string> fileNames;
	ConstraintRotationNames constraintRotationNames;
	po::variables_map vm;

	parseOptions (argc, argv, 
		      &t1sFile, &fileNames, &constraintRotationNames,
		      &vm);
	foamAlongTime.ParseFiles (
	    fileNames,
	    vm.count (optionName[Option::USE_ORIGINAL]),
	    constraintRotationNames,
	    vm.count (optionName[Option::DEBUG_PARSING]), 
	    vm.count (optionName[Option::DEBUG_SCANNING]));
	size_t timeSteps = foamAlongTime.GetTimeSteps ();
	if (vm.count (optionName[Option::T1S]))
	    foamAlongTime.ReadT1s (t1sFile, timeSteps);
        if (timeSteps != 0)
        {
	    foamAlongTime.SetAdjustPressure (
		! vm.count (optionName[Option::ORIGINAL_PRESSURE]));
	    foamAlongTime.Preprocess ();
	    if (vm.count (optionName[Option::OUTPUT_TEXT]))
		cdbg << foamAlongTime;
	    else
	    {
		int result;
		QCoreApplication::setOrganizationName ("Swansea University");
		QCoreApplication::setOrganizationDomain ("www.swansea.ac.uk");
		QCoreApplication::setApplicationName ("FoamVis");
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
