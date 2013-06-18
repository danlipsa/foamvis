/**
 * @file   BrowseSimulations.h
 * @author Dan R. Lipsa
 * @date 18 August 2011
 * @brief Dialog used for choosing simulations to analyse or compare.
 */

#ifndef __BROWSE_SIMULATION_H__
#define __BROWSE_SIMULATION_H__

#include "ui_BrowseSimulations.h"
#include "Labels.h"

/**
 * @brief Dialog used for choosing simulations to analyse or compare.
 */
class BrowseSimulations : 
    public QDialog, private Ui::BrowseSimulations
{
public:
    BrowseSimulations (
	const char* imageFolder, 
        const vector<string>& names, const vector<size_t>& questionMarkCount,
	const vector<Labels>& labels, QWidget* parent = 0);
    vector<size_t> GetSelectedIndexes () const;
    vector<string> GetFilter () const;
    vector<size_t> GetQuestionMarkCount ();

public Q_SLOTS:
    void CurrentChangedSimulation (int current, int previous);
    void SelectionChangedSimulation (const QItemSelection & selected, 
                                     const QItemSelection & deselected);
    void CurrentIndexChangedLabel(QString);
    void TextChangedLineEdit (QString);

private:
    size_t globalIndex (size_t localIndex) const;
    string getInitialFilter (const vector<size_t>& count);
    vector<size_t> getQuestionMarkCount (const vector<size_t>& indexesSelected);
    vector<size_t> indexesSelected (const QModelIndexList& mil);
    void setLineEditFilter (const vector<size_t>& questionMarkCount);

private:
    static const char* LABEL_ALL;

private:
    Q_OBJECT
    QStringListModel m_model;    
    QStringList m_selectedNames;
    vector<size_t> m_selectedQuestionMarkCount;
    string m_imageFolder;
    const vector<string>& m_names;
    const vector<size_t>& m_questionMarkCount;
    const vector<Labels>& m_labels;
};


#endif //__BROWSE_SIMULATION_H__

// Local Variables:
// mode: c++
// End:
