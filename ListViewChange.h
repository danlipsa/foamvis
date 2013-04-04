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
	return QListView::selectedIndexes ();
    }


Q_SIGNALS:
    void CurrentChanged (int current, int previous);
    void SelectionChanged (const QItemSelection & selected, 
                           const QItemSelection & deselected);

protected:
    virtual void currentChanged (const QModelIndex& current, 
				 const QModelIndex& previous)
    {
	QListView::currentChanged (current, previous);
	Q_EMIT CurrentChanged (current.row (), previous.row ());
    }

    virtual void selectionChanged (const QItemSelection & selected, 
                                   const QItemSelection & deselected)
    {
        QListView::selectionChanged (selected, deselected);
        Q_EMIT SelectionChanged (selected, deselected);
    }


private:
    Q_OBJECT
};

#endif //__LIST_VIEW_CHANGE_H__

// Local Variables:
// mode: c++
// End:
