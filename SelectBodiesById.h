/**
 * @file   SelectBodiesById.h
 * @author Dan R. Lipsa
 * @date 4 Dec. 2010
 * @brief Dialog for selecting a list of bubbles using their ID.
 * @ingroup ui
 */

#ifndef __SELECT_BODIES_BY_ID_H__
#define __SELECT_BODIES_BY_ID_H__

#include "ui_SelectBodiesById.h"

/**
 * @brief Dialog for selecting a list of bubbles using their ID.
 */
class SelectBodiesById : 
    public QDialog, private Ui::SelectBodiesById
{
public:
    SelectBodiesById (QWidget* parent);
    void Init (size_t minBodyId, size_t maxBodyId);

    const vector<size_t>& GetIds () const
    {
	return m_ids;
    }
    void SetMaxBodyId (size_t maxBodyId)
    {
	m_maxBodyId = maxBodyId;
    }
    void SetMinBodyId (size_t minBodyId)
    {
	m_minBodyId = minBodyId;
    }
    void UpdateLabelMinMax ();
    void ClearEditIds ();

public Q_SLOTS:
    virtual void accept ();
    int exec ();

private:
    Q_OBJECT
    vector<size_t> m_ids;
    size_t m_maxBodyId;
    size_t m_minBodyId;
};

#endif //__SELECT_BODIES_BY_ID_H__

// Local Variables:
// mode: c++
// End:
