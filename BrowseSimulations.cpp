/**
 * @file   BrowseSimulations.cpp
 * @author Dan R. Lipsa
 * @date 18 August 2011
 *
 * Definition of the BrowseSimulations class
 */

#include "BrowseSimulations.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Utils.h"

const char* BrowseSimulations::LABEL_ALL = "All";

BrowseSimulations::BrowseSimulations (
    const vector<string>& names, const vector<Labels>& labels, QWidget* parent) :
    QDialog (parent), m_names (names), m_labels (labels)
{
    setupUi (this);
    m_selectedNames = ToQStringList (names);
    m_model.setStringList (m_selectedNames);
    listViewSimulation->setModel (&m_model);
    lineEditFilter->setText ("0001");
    QModelIndex index = m_model.index (0);
    listViewSimulation->setCurrentIndex (index);
    comboBoxLabel->addItem (LABEL_ALL);
    set<string> distinctLabels;
    BOOST_FOREACH (Labels labels, m_labels)
    {
	BOOST_FOREACH (string label, labels.m_values)
	    distinctLabels.insert (label);
    }
    BOOST_FOREACH (string label, distinctLabels)
	comboBoxLabel->addItem (label.c_str ());
}

size_t BrowseSimulations::GetIndex () const
{
    size_t index = listViewSimulation->currentIndex ().row ();
    string name = m_selectedNames[index].toStdString ();
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
	"simulations/" + m_selectedNames[row] + ".jpg";
    QPixmap pixmap (fileName);
    if (pixmap.isNull ())
	labelImage->setText ("Invalid file: " + fileName);
    else
	labelImage->setPixmap (pixmap);
    update ();
}


void BrowseSimulations::CurrentIndexChangedLabel(QString label)
{
    m_selectedNames.clear ();
    if (label == LABEL_ALL)
    {
	m_model.setStringList (ToQStringList (m_names));
	BOOST_FOREACH (string name, m_names)
	    m_selectedNames << name.c_str ();
    }
    else
    {
	for (size_t i = 0; i < m_names.size (); ++i)
	{
	    Labels labels = m_labels[i];
	    if (std::find (labels.m_values.begin (), labels.m_values.end (), 
			   label.toStdString ()) != labels.m_values.end ())
		m_selectedNames << m_names[i].c_str ();
	}
	m_model.setStringList (m_selectedNames);
    }
    update ();
    listViewSimulation->selectionModel ()->select (
	m_model.index (0), QItemSelectionModel::Select);
    CurrentChangedSimulation (0);
    listViewSimulation->setFocus (Qt::OtherFocusReason);
}
