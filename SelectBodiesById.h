/**
 * @file   SelectBodiesById.h
 * @author Dan R. Lipsa
 * @date 4 Dec. 2010
 *
 * Declaration of the SelectBodiesById
 */

#ifndef __SELECT_BODIES_BY_ID_H__
#define __SELECT_BODIES_BY_ID_H__

#include "ui_SelectBodiesById.h"

class SelectBodiesById : 
    public QDialog, private Ui::SelectBodiesById
{
public:
    SelectBodiesById (QWidget* parent);
    const vector<size_t>& GetIds () const
    {
	return m_ids;
    }

public Q_SLOTS:
    virtual void accept ();

private:
    Q_OBJECT
    vector<size_t> m_ids;
};

#endif //__SELECT_BODIES_BY_ID_H__

// Local Variables:
// mode: c++
// End:
