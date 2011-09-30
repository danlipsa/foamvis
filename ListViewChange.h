/**
 * @file   ListViewChange.h
 * @author Dan R. Lipsa
 * @date 20 August 2010
 *
 * Declaration of the ListViewChange class
 */

#ifndef __LIST_VIEW_CHANGE_H__
#define __LIST_VIEW_CHANGE_H__

class ListViewChange : public QListView
{
public:
    ListViewChange (QWidget* parent = 0) :
	QListView (parent)
    {
    }
    QModelIndexList selectedIndexes () const
    {
	return selectedIndexes ();
    }


Q_SIGNALS:
    void CurrentChanged (int current);

protected:
    virtual void currentChanged (const QModelIndex& current, 
				 const QModelIndex& previous)
    {
	QListView::currentChanged (current, previous);
	Q_EMIT CurrentChanged (current.row ());
    }

private:
    Q_OBJECT
};

#endif //__LIST_VIEW_CHANGE_H__

// Local Variables:
// mode: c++
// End:
