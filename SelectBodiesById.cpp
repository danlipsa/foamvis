/**
 * @file   SelectBodiesById.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the SelectBodiesById class
 */

#include "SelectBodiesById.h"
#include "DebugStream.h"

SelectBodiesById::SelectBodiesById (QWidget* parent) :
    QDialog (parent)
{
    setupUi (this);
    QRegExp rx("(\\d+ +)*\\d+");
    QValidator *validator = new QRegExpValidator(rx, this);
    lineEditIds->setValidator(validator);
}

void SelectBodiesById::setBodyCount (size_t bodyCount)
{
    m_bodyCount = bodyCount;
    ostringstream instructions;
    instructions << labelInstructions->text ().toAscii ().data ()
		 << (bodyCount - 1) << ".";
    labelInstructions->setText (instructions.str ().c_str ());    
}

void SelectBodiesById::accept ()
{
    QString text = lineEditIds->text ();
    QStringList bodyIds = text.split (' ', QString::SkipEmptyParts);
    m_ids.clear ();
    BOOST_FOREACH (QString bodyId, bodyIds)
    {
	size_t value = bodyId.toUInt ();
	if (value >= m_bodyCount)
	{
	    ostringstream ostr;
	    ostr << "Body ID: " << value << " greater than the number of bodies";
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
