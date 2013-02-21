/**
 * @file   SelectBodiesById.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the SelectBodiesById class
 */

#include "SelectBodiesById.h"
#include "Debug.h"

SelectBodiesById::SelectBodiesById (QWidget* parent) :
    QDialog (parent)
{
    setupUi (this);
    QRegExp rx("(\\d+ +)*\\d+");
    QValidator *validator = new QRegExpValidator(rx, this);
    lineEditIds->setValidator(validator);
}

void SelectBodiesById::Init (size_t minBodyId, size_t maxBodyId)
{
    SetMinBodyId (minBodyId);
    SetMaxBodyId (maxBodyId);
    UpdateLabelMinMax ();
}


void SelectBodiesById::UpdateLabelMinMax ()
{
    ostringstream instructions;
    instructions << labelInstructions->text ().toStdString ()
		 << m_minBodyId << " to " << m_maxBodyId << ".";
    labelInstructions->setText (instructions.str ().c_str ());    
}

void SelectBodiesById::ClearEditIds ()
{
    lineEditIds->setText ("");
}

void SelectBodiesById::accept ()
{
    QString text = lineEditIds->text ();
    QStringList bodyIds = text.split (' ', QString::SkipEmptyParts);
    m_ids.clear ();
    BOOST_FOREACH (QString bodyId, bodyIds)
    {
	size_t value = bodyId.toUInt ();
	if (value < m_minBodyId || value > m_maxBodyId)
	{
	    ostringstream ostr;
	    ostr << "Body ID: " << value << " outside body id range.";
	    QMessageBox messageBox;
	    messageBox.setText (ostr.str ().c_str ());
	    messageBox.exec ();
	    return;
	}
	m_ids.push_back (value);
    }
    QDialog::accept ();
}

int SelectBodiesById::exec ()
{
    lineEditIds->selectAll ();
    return QDialog::exec ();
}
