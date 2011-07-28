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
#include "Force.h"
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
 * \subsection sec_onbody Executed when creating a body (may create duplicate 
   faces)
 *
<pre>
Add all adjacent faces of face 0 to a queue.
while (no more items in the queue)
{
   remove a adjacent face, translate it if needed and mark it visited.
   if several faces fit, choose the face with smallest angle between its 
   normal and the original face normal.
}

</pre>
 *
 * \section sec_physical_tesselation Physical and tesselation edges and vertices
 * In 2D we don't have physical edges. A vertex is "physical" if has
 * >= 3 edges adjacent to it.
 *
 * In 3D, an edge is physical if it has 6 AdjacentOrientedFace is part of. (is
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
    "constraint",
    "constraint-rotation",
    "debug-parsing",
    "debug-scanning",
    "dmp-files",
    "forces",
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
        CONSTRAINT,
	CONSTRAINT_ROTATION,
	DEBUG_PARSING,
	DEBUG_SCANNING,
	DMP_FILES,
	FORCES,
	HELP,
	ORIGINAL_PRESSURE,
	OUTPUT_TEXT,
	T1S,
	USE_ORIGINAL,
	VERSION,
	COUNT
    };
};

class ReadStringToken
{
public:
    ReadStringToken (boost::tokenizer<>& tok, const string& errorMessage) :
	m_tok (tok), m_errorMessage (errorMessage)
    {
    }

    void operator() (string* dest, boost::tokenizer<>::iterator* it)
    {
	if (++(*it) == m_tok.end ())
	    throw invalid_argument (m_errorMessage);
	*dest = *(*it);
    }
private:
    boost::tokenizer<>& m_tok;
    const string& m_errorMessage;
};

void validate(boost::any& v, const std::vector<std::string>& values,
              ConstraintRotationNames* ignore1, int ignore2)
{
    (void) ignore1;(void)ignore2;
    ConstraintRotationNames crn;
    boost::tokenizer<> tok (values[0]);
    istringstream istr;
    string errorMessage ("--constraint-rotation needs four parameters.");
    boost::tokenizer<>::iterator it = tok.begin ();
    if (it == tok.end ())
	throw invalid_argument (errorMessage);
    istr.str (*it);
    istr >> crn.m_constraintIndex;
    ReadStringToken readStringToken (tok, errorMessage);
    readStringToken (&crn.m_xName, &it);
    readStringToken (&crn.m_yName, &it);
    readStringToken (&crn.m_angleName, &it);
    v = boost::any(crn);
}


void validate(boost::any& v, const std::vector<std::string>& values,
              ForceNames* ignore1, int ignore2)
{
    (void)ignore1;(void)ignore2;
    ForceNames fn;
    boost::tokenizer<> tok (values[0]);
    istringstream istr;
    string errorMessage ("--forces needs 5 parameters.");
    boost::tokenizer<>::iterator it = tok.begin ();
    if (it == tok.end ())
	throw invalid_argument (errorMessage);
    istr.str (*it);
    istr >> fn.m_bodyId;
    --fn.m_bodyId;
    ReadStringToken readStringToken (tok, errorMessage);
    readStringToken (&fn.m_networkForceName[0], &it);
    readStringToken (&fn.m_networkForceName[1], &it);
    readStringToken (&fn.m_pressureForceName[0], &it);
    readStringToken (&fn.m_pressureForceName[1], &it);
    v = boost::any(fn);    
}

void parseOptions (int argc, char *argv[],
		   string* t1sFile,
		   vector<string>* fileNames,
		   ConstraintRotationNames* constraintRotationNames,
		   vector<ForceNames>* forcesNames,
		   po::variables_map* vm)
{
    // Declare the supported options.
    po::options_description genericOptions(
	"foam [OPTIONS] <files> ...\n"
	"<files> - one or more DMP files\n"
	"OPTIONS");
    genericOptions.add_options()
	(optionName[Option::CONSTRAINT],
	 po::value<size_t>(&constraintRotationNames->m_constraintIndex), 
	 "a constraint that specifies an object.\n"
         "arg=<constraint> where <constraint> is the constraint number.")
	(optionName[Option::CONSTRAINT_ROTATION], 
	 po::value<ConstraintRotationNames>(constraintRotationNames), 
	 "a constraint that specifies an object which rotates.\n"
	 "arg=\"<constraint> <xName> <yName> <angleName>\" where " 
	 "<constraint> specifies the constraint number, <xName>, <yName> "
	 "specify names for parameters that store the center of rotation and "
	 "<angleName> specifies the name of the parameter that stores "
	 "the rotation angle.")
	(optionName[Option::DEBUG_PARSING], 
	 "produces output that help debugging the parser")
	(optionName[Option::DEBUG_SCANNING], 
	 "produces output that helps debugging the scanner")
	(optionName[Option::FORCES], 
	 po::value< vector<ForceNames> >(forcesNames),
	 "reads the forces acting on a body.\n"
	 "arg=\"<bodyId> <networkXName> <networkYName> "
	 "<pressureXName> <pressureYName>\" where <bodyId> is the ID of the "
	 "body the force acts on, (<networkXName>, <networkYName>) are the "
	 "names of the X and Y components of the network force and "
	 "(<pressureXName>, <pressureYName>) are the X and Y components of "
	 "the pressure force.")
	(optionName[Option::HELP], "produce help message")
	(optionName[Option::ORIGINAL_PRESSURE],
	 "shows original pressure values")	    
	(optionName[Option::OUTPUT_TEXT],
	 "outputs a text representation of the data")	    
	(optionName[Option::T1S],
	 po::value<string>(t1sFile), 
	 "reads T1 positions.\n"
	 "arg=<file> where <file> specifies a text file with "
	 "T1 times and positions. Reading T1s won't work if you "
	 "skip time steps")
	(optionName[Option::USE_ORIGINAL], "uses the ORIGINAL atribute "
	 "to figure out the body id.")
	(optionName[Option::VERSION], "prints version information")
	;
    po::options_description hiddenOptions("Hidden options");
    hiddenOptions.add_options()
	(optionName[Option::DMP_FILES], 
	 po::value< vector<string> >(fileNames), "dmp file");
    po::options_description options;
    options.add(genericOptions).add(hiddenOptions);

    po::positional_options_description positionalOptions;
    positionalOptions.add(optionName[Option::DMP_FILES], -1);

    po::store(po::command_line_parser (argc, argv).
	      options (options).positional (positionalOptions).
	      run (), *vm);
    po::notify(*vm);
    if (constraintRotationNames->m_constraintIndex != INVALID_INDEX)
	--constraintRotationNames->m_constraintIndex;
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
	const char* version = "0.5.2062";
	cout << "foam version " << version;
#ifdef QT_NO_DEBUG
	cout << " RELEASE";
#else
	cout << " DEBUG";
#endif
	cout << endl;
	exit (0);
    }
    if (argc == 1 || ! vm->count (optionName[Option::DMP_FILES]))
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
	vector<ForceNames> forcesNames;
	po::variables_map vm;

	parseOptions (argc, argv, 
		      &t1sFile, &fileNames, &constraintRotationNames,
		      &forcesNames,
		      &vm);	
	foamAlongTime.ParseFiles (
	    fileNames,
	    vm.count (optionName[Option::USE_ORIGINAL]),
	    constraintRotationNames,
	    forcesNames,
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
