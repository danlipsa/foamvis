/**
 * @file   SelectBodiesById.cpp
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Definition of the SelectBodiesById class
 */

#include "SelectBodiesById.h"

SelectBodiesById::SelectBodiesById (QWidget* parent) :
    QDialog (parent)
{
    setupUi (this);
    QRegExp rx("(\\d *, *)*\\d");
    QValidator *validator = new QRegExpValidator(rx, this);
    lineEditIds->setValidator(validator);
}

void SelectBodiesById::accept ()
{
    QRegExp rx ("\\d");
    QString text = lineEditIds->text ();
    int pos = 0;
    while ((pos = rx.indexIn (text, pos)) != -1)
    {
	pos += rx.matchedLength ();
    }
    QDialog::accept ();
}
