/**
 * @file   BrowseSimulations.cpp
 * @author Dan R. Lipsa
 * @date 18 August 2011
 *
 * Definition of the BrowseSimulations class
 */

#include "BrowseSimulations.h"
#include "DebugStream.h"
#include "Utils.h"

BrowseSimulations::BrowseSimulations (const vector<string>& names, 
				      QWidget* parent) :
    QDialog (parent), m_names (names)
{
    setupUi (this);
    m_model.setStringList (ToQStringList (names));
    listViewSimulation->setModel (&m_model);
    lineEditFilter->setText ("0001");
    QModelIndex index = m_model.index (0);
    listViewSimulation->setCurrentIndex (index);
}

void BrowseSimulations::CurrentChanged (int row)
{
    string fileName = m_names[row] + ".jpg";
    QPixmap pixmap (fileName.c_str ());
    if (pixmap.isNull ())
	labelImage->setText (string ("Invalid file: " + fileName).c_str ());
    else
	labelImage->setPixmap (pixmap);
    update ();
}

size_t BrowseSimulations::GetIndex () const
{
    return listViewSimulation->currentIndex ().row ();
}

string BrowseSimulations::GetFilter () const
{
    return string (lineEditFilter->text ().toAscii ().constData ());
}
