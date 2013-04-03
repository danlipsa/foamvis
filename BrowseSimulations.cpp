/**
 * @file   BrowseSimulations.cpp
 * @author Dan R. Lipsa
 * @date 18 August 2011
 *
 * Definition of the BrowseSimulations class
 */

#include "BrowseSimulations.h"
#include "Debug.h"
#include "Utils.h"

const char* BrowseSimulations::LABEL_ALL = "All";

BrowseSimulations::BrowseSimulations (
    const char* imageFolder,
    const vector<string>& names, const vector<size_t>& questionMarkCount,
    const vector<Labels>& labels, 
    QWidget* parent) :

    QDialog (parent), m_imageFolder (imageFolder), 
    m_names (names), m_questionMarkCount (questionMarkCount), m_labels (labels)
{
    setupUi (this);
    m_selectedNames = ToQStringList (names);
    m_selectedQuestionMarkCount = m_questionMarkCount;
    m_model.setStringList (m_selectedNames);
    QModelIndex index = m_model.index (0);
    listViewSimulation->setSelectionMode (QAbstractItemView::ExtendedSelection);
    listViewSimulation->setModel (&m_model);
    listViewSimulation->setCurrentIndex (index);
    comboBoxLabel->addItem (LABEL_ALL);
    lineEditFilter->setToolTip (
        "? matches any single character<br>"
        "[...] matches any character between the brackets<br>"
        "any other character represents itself<br>"
        "the number of characters has to stay the same");
    set<string> distinctLabels;
    BOOST_FOREACH (Labels labels, m_labels)
    {
	BOOST_FOREACH (string label, labels.m_values)
	    distinctLabels.insert (label);
    }
    BOOST_FOREACH (string label, distinctLabels)
	comboBoxLabel->addItem (label.c_str ());
}


vector<size_t> BrowseSimulations::GetSelectedIndexes () const
{
    vector<size_t> selectedIndexes;
    QModelIndexList mil = listViewSimulation->selectedIndexes ();
    Q_FOREACH (QModelIndex mi, mil)
	selectedIndexes.push_back (globalIndex (mi.row ()));
    return selectedIndexes;
}

size_t BrowseSimulations::globalIndex (size_t localIndex) const
{
    string name = m_selectedNames[localIndex].toStdString ();
    vector<string>::const_iterator it = 
	std::find (m_names.begin (), m_names.end (), name);
    RuntimeAssert (it != m_names.end (), 
		   "BrowseSimulations::GetIndex invalid index");
    return it - m_names.begin ();
}

string BrowseSimulations::GetFilter () const
{
    return string (lineEditFilter->text ().toAscii ().constData ());
}

// Slots
// ======================================================================

void BrowseSimulations::CurrentChangedSimulation (int row)
{
    QString fileName = 
	QString (m_imageFolder.c_str ()) + "/" + m_selectedNames[row] + ".jpg";
    QPixmap pixmap (fileName);
    if (pixmap.isNull ())
	labelImage->setText ("Invalid file: " + fileName);
    else
	labelImage->setPixmap (pixmap);
    lineEditFilter->setText (
        getInitialFilter (m_selectedQuestionMarkCount[row]).c_str ());
    update ();
}

string BrowseSimulations::getInitialFilter (size_t count)
{
    ostringstream ostr;
    for (size_t i = 0; i < count - 1; ++i)
        ostr << "0";
    ostr << "1";
    return ostr.str ();
}

void BrowseSimulations::CurrentIndexChangedLabel(QString label)
{
    m_selectedNames.clear ();
    if (label == LABEL_ALL)
    {
	m_model.setStringList (ToQStringList (m_names));
        m_selectedQuestionMarkCount.resize (m_names.size ());
        for (size_t i = 0; i < m_names.size (); ++i)
        {
	    m_selectedNames << m_names[i].c_str ();
            m_selectedQuestionMarkCount[i] = m_questionMarkCount[i];
        }
    }
    else
    {
	for (size_t i = 0; i < m_names.size (); ++i)
	{
	    Labels labels = m_labels[i];
	    if (std::find (labels.m_values.begin (), labels.m_values.end (), 
			   label.toStdString ()) != labels.m_values.end ())
            {
		m_selectedNames << m_names[i].c_str ();
                m_selectedQuestionMarkCount.push_back (m_questionMarkCount[i]);
            }
	}
	m_model.setStringList (m_selectedNames);
    }
    update ();
    listViewSimulation->selectionModel ()->select (
	m_model.index (0), QItemSelectionModel::Select);
    CurrentChangedSimulation (0);
    listViewSimulation->setFocus (Qt::OtherFocusReason);
}
