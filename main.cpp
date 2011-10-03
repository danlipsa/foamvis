/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Parses an Evolver DMP file and displays the data from the file.
 */
#include "Application.h"
#include "BrowseSimulations.h"
#include "Options.h"
#include "Debug.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "Force.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "DebugStream.h"

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

    size_t simulationsCount = clo->m_simulationIndexes.size ();
    if (simulationsCount == 0)
    {
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
	    (*co)[i]->read (clo->m_parametersArray[clo->m_simulationIndexes[i]],
			 clo->m_filter);
	}
    }    
}


void parseOptions (int argc, char *argv[], 
		   FoamAlongTimeGroup* foamAlongTimeGroup, bool* print)
{
    CommandLineOptions clo;
    vector< boost::shared_ptr<CommonOptions> > co;
    readOptions (argc, argv, &clo, &co);
    size_t simulationsCount = co.size ();
    foamAlongTimeGroup->SetSize (simulationsCount);
    for (size_t i = 0; i < simulationsCount; ++i)
    {
	FoamAlongTime& foamAlongTime = foamAlongTimeGroup->GetFoamAlongTime (i);
	if (co[i]->m_vm.count (Option::m_name[Option::T1S]))
	    foamAlongTime.ParseT1s (
		co[i]->m_t1sFile, co[i]->m_ticksForTimeStep,
		co[i]->m_vm.count (Option::m_name[Option::T1S_LOWER]));
	foamAlongTime.ParseDMPs (
	    co[i]->m_fileNames, 
	    co[i]->m_vm.count (Option::m_name[Option::USE_ORIGINAL]),
	co[i]->m_dmpObjectInfo, co[i]->m_forcesNames,
	    co[i]->m_vm.count (Option::m_name[Option::DEBUG_PARSING]), 
	    co[i]->m_vm.count (Option::m_name[Option::DEBUG_SCANNING]));
	if (foamAlongTime.GetTimeSteps () == 0)
	{
	    cdbg << "Error: The patern provided does not match any file" 
		 << endl;
	    exit (13);
	}
	
	foamAlongTime.SetAdjustPressure (
	    ! co[i]->m_vm.count (Option::m_name[Option::ORIGINAL_PRESSURE]));
	foamAlongTime.Preprocess ();
	*print = co[i]->m_vm.count (Option::m_name[Option::OUTPUT_TEXT]);
    }
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
    boost::shared_ptr<Application> app = Application::Get (
	argc, argv);
    try
    {
	FoamAlongTimeGroup foamAlongTimeGroup;
	bool print;
	parseOptions (argc, argv, &foamAlongTimeGroup, &print);
	if (print)
	    cdbg << foamAlongTimeGroup;
	else
	{
	    int result;
	    MainWindow window (foamAlongTimeGroup);
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
