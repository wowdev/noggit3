// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <helper/qt/non_recursive_filter_model.h>

namespace helper
{
  namespace qt
  {
    non_recursive_filter_model::non_recursive_filter_model (QObject* parent)
      : QSortFilterProxyModel (parent)
    { }

    bool non_recursive_filter_model::filterAcceptsRow ( int source_row
                                                      , const QModelIndex &source_parent
                                                      ) const
    {
      return is_shown (sourceModel()->index (source_row, 0, source_parent));
    }

    bool non_recursive_filter_model::is_shown (const QModelIndex& index) const
    {
      if (sourceModel()->rowCount (index) == 0)
      {
        return sourceModel()->data ( sourceModel()->index ( index.row()
                                                          , 0
                                                          , index.parent()
                                                          )
                                   , Qt::DisplayRole
                                   ).toString().contains (filterRegExp());
      }
      else
      {
        for (int child (0); child < sourceModel()->rowCount (index); ++child)
        {
          const QModelIndex child_index (sourceModel()->index (child, 0, index));
          if (child_index.isValid() && is_shown (child_index))
          {
            return true;
          }
        }
      }

      return false;
    }
  }
}
