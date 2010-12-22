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

void SelectBodiesById::accept ()
{
    QString text = lineEditIds->text ();
    QStringList bodyIds = text.split (' ', QString::SkipEmptyParts);
    BOOST_FOREACH (QString bodyId, bodyIds)
    {
	size_t value = bodyId.toUInt ();
	m_ids.push_back (value);
	cdbg << value << endl;
    }
    QDialog::accept ();
}

int SelectBodiesById::exec ()
{
    lineEditIds->selectAll ();
    return QDialog::exec ();
}
