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

class BrowseSimulations : 
    public QDialog, private Ui::BrowseSimulations
{
public:
    BrowseSimulations (const vector<string>& names, QWidget* parent = 0);
    size_t GetIndex () const;
    string GetFilter () const;

public Q_SLOTS:
    void CurrentChanged (int row);

private:
    Q_OBJECT
    QStringListModel m_model;
    vector<string> m_names;
};


#endif //__BROWSE_SIMULATION_H__

// Local Variables:
// mode: c++
// End:
