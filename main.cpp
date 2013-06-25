/**
 * @file main.cpp
 * @author Dan R. Lipsa
 * @ingroup parser
 * @brief Drives parsing of SE .dmp files
 */

// ----------------------------------------------------------------------
/*
 * @todo Look at paths coresponding to bubbles around the circulation
 *       shown by average: Simon's email 21 Sep. 2012
 * @todo Add vorticity: Simon's email 21 Sep. 2012
 * @todo Implement VIEW_TRANSFORM, VIEW_TRANSFORM_GENERATORS, TRANSFORM_EXPR for
 *       twogascoarse dataset.
 * @todo Why time displacement does not go all the way to max Z?
 * @todo Use vertex arrays for center paths
 * @todo Use google-breakpad for reporting crashes.  
 * @todo Set new line significant only for list of vertices, edges,
 *       faces, bodies
 * @todo Replace vector + tensor drawing with geometry shader or CPU 
 *       (instead fragment shader)
 * @todo Optimization: for 2D foam, store vector count in the same texture
 *       instead of storing it in the scalar texture.
 * @todo Bug: velocity average shows a black blob if only one time step of data 
         is loaded. An error message should be shown instead.
 * @todo Bug: fix Overlay, Velocity, Glyph, Grid Cell Center.
 * @todo Bug: Translate grid does not work with streamlines.
 * @todo Fix the (slow) movement of the focus in context view for the GL view.
 * @todo Replace glScale with camera movement for GL view.
 * @todo Implement clamping such that the number of colors are not reduced
 *       for small clamp intervals.
 * @todo Add an option for 3D RegularGridAverage for AverageCountType::LOCAL. 
 *       This would be useful for shear160dumps_3b where a voxel does not have
 *       a value in every time steps.
 * @todo Use array indexes instead of pointers for faces part of a
 *       body, and vertices part of a face.
 *       This works better with VTK and with OpenGL vertex arrays.
 */

// ----------------------------------------------------------------------
/**
 * @mainpage FoamVis 
 *
 * FoamVis provides techniques for visualization, exploration and
 * analysis of time-dependent foam simulation data generated by
 * Surface Evolver. For requirements and instructions on how to
 * compile and execute the program see the @subpage readme file. For a
 * release log see the @subpage log file. For a high level overview of
 * the design and implementation of the program see the list of <a
 * href="modules.html">Modules</a>.
 *
 * @section download Download
 * - <a href="http://csgalati.swansea.ac.uk/foam/build">Sources</a> for
 * FoamVis and required libraries.
 * - Mac Os X <a href="http://csgalati.swansea.ac.uk/foam/bin/mac-osx-10.6.8/">Snow Leopard</a> binary release (not the latest version)
 * 
 */

// ----------------------------------------------------------------------
/** 
 * @page readme ReadMe
 * @verbinclude README.txt
 */

// ----------------------------------------------------------------------
/** 
 * @page log Release Log
 * @verbinclude LOG.txt
 */


#include "Application.h"
#include "BrowseSimulations.h"
#include "Options.h"
#include "Debug.h"
#include "Foam.h"
#include "Simulation.h"
#include "ForceOneObject.h"
#include "ParsingData.h"
#include "MainWindow.h"

void readOptions (int argc, char *argv[],
		  CommandLineOptions* clo, 
		  vector< boost::shared_ptr<CommonOptions> >* co)
{
    try
    {
	clo->read (argc, argv);
    }
    // ignore if there no command line options, we'll read common options
    catch (const exception& e)
    {}

    try
    {
	size_t simulationsCount = clo->m_simulationIndexes.size ();
	if (simulationsCount == 0)
	{
	    if (! clo->m_iniFileName.empty ())
		// the user pressed cancel on the BrowseSimulations dialog
		exit (0);
	    simulationsCount = 1;
	    co->resize (1);
	    // read common options from the command line
	    (*co)[0].reset (new CommonOptions ());
	    (*co)[0]->read (argc, argv);
	}
	else
	{
	    // read common options from the ini file
	    co->resize (simulationsCount);
	    for (size_t i = 0; i < simulationsCount; ++i)
	    {
		string simulation = clo->m_names[clo->m_simulationIndexes[i]];
		cdbg << "Simulation name: " << simulation << " ...\n";
		(*co)[i].reset (new CommonOptions ());
		(*co)[i]->read (
		    clo->m_parametersArray[clo->m_simulationIndexes[i]],
		    clo->m_filter[i]);
	    }
	}
    }
    catch (const exception& e)
    {
	cdbg << "Exception reading common options: " << e.what () << endl;
	exit (13);
    }
}


void parseOptions (
    int argc, char *argv[], 
    boost::shared_ptr<SimulationGroup> simulationGroup, bool* outputText)
{
    CommandLineOptions clo;
    vector< boost::shared_ptr<CommonOptions> > co;
    readOptions (argc, argv, &clo, &co);
    size_t simulationsCount = co.size ();
    simulationGroup->SetSize (simulationsCount);
    for (size_t i = 0; i < simulationsCount; ++i)
    {
	Simulation& simulation = simulationGroup->GetSimulation (i);
        simulation.SetBoundingBoxAllTimeSteps (co[i]->m_simulationBoundingBoxAllTimeSteps);
	if (co[i]->m_vm.count (Option::m_name[Option::RESOLUTION]))
	    simulation.SetRegularGridResolution (co[i]->m_resolution);
	if (co[i]->m_vm.count (Option::m_name[Option::T1S]))
	    simulation.ParseT1s (
		co[i]->m_t1sFile, co[i]->m_ticksForTimeStep,
		co[i]->m_vm.count (Option::m_name[Option::T1S_LOWER]));
	simulation.ParseDMPs (
	    co[i]->m_fileNames, 
	    co[i]->m_vm.count (Option::m_name[Option::USE_ORIGINAL]),
	    co[i]->m_dmpObjectInfo, co[i]->m_forceNames,
	    clo.m_vm.count (Option::m_name[Option::DEBUG_PARSING]), 
	    clo.m_vm.count (Option::m_name[Option::DEBUG_SCANNING]));
	string simulationName = clo.m_names.empty () ?
	    LastDirFile (co[0]->m_fileNames[0].c_str ()) : 
            clo.m_names[clo.m_simulationIndexes[i]];
	simulation.SetName (simulationName);
	simulation.SetRotation2D (co[i]->m_rotation2D);
	simulation.SetReflectionAxis (co[i]->m_reflectionAxis);
	if (simulation.GetTimeSteps () == 0)
	{
	    cdbg << "Error: The patern provided does not match any file" 
		 << endl;
	    exit (13);
	}
	
	simulation.SetPressureAdjusted (
	    ! co[i]->m_vm.count (Option::m_name[Option::ORIGINAL_PRESSURE]));
	simulation.Preprocess ();
    }
    *outputText = clo.m_vm.count (Option::m_name[Option::OUTPUT_TEXT]);
}


/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName ("Swansea University");
    QCoreApplication::setOrganizationDomain ("www.swansea.ac.uk");
    QCoreApplication::setApplicationName ("FoamVis");
    /*cdbg << "min: " << numeric_limits<double>::min ()
	 << " max: " << numeric_limits<double>::max () << endl;
    */

    boost::shared_ptr<Application> app = Application::Get (argc, argv);
    bool outputText;
    try
    {
        boost::shared_ptr<SimulationGroup> simulationGroup (
            new SimulationGroup ());
	parseOptions (argc, argv, simulationGroup, &outputText);
	if (outputText)
	    cdbg << simulationGroup;
	else
	{
	    int result;
	    MainWindow window (simulationGroup);
	    window.show();
	    result = app->exec();
	    app->release ();
	    return result;
	}
	return 0;
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
	exit (13);
    }
}
