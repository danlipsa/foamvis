/**
 * @file   BrowseSimulation.h
 * @author Dan R. Lipsa
 * @date 18 August 2011
 *
 * Declaration of the BrowseSimulation class
 */

#ifndef __BROWSE_SIMULATION_H__
#define __BROWSE_SIMULATION_H__

#include "ui_BrowseSimulations.h"

struct Labels
{
    vector<string> m_values;
};

class BrowseSimulations : 
    public QDialog, private Ui::BrowseSimulations
{
public:
    BrowseSimulations (
	const char* imageFolder, const vector<string>& names, 
	const vector<Labels>& labels, QWidget* parent = 0);
    size_t GetIndex () const;
    string GetFilter () const;

public Q_SLOTS:
    void CurrentChangedSimulation (int row);
    void CurrentIndexChangedLabel(QString);

private:
    static const char* LABEL_ALL;

private:
    Q_OBJECT
    QStringListModel m_model;    
    QStringList m_selectedNames;
    string m_imageFolder;
    const vector<string>& m_names;
    const vector<Labels>& m_labels;
};


#endif //__BROWSE_SIMULATION_H__

// Local Variables:
// mode: c++
// End:
