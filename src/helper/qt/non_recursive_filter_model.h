#ifndef __HELPER_QT_NON_RECURSIVE_FILTER_MODEL_H
#define __HELPER_QT_NON_RECURSIVE_FILTER_MODEL_H

#include <QSortFilterProxyModel>

namespace helper
{
  namespace qt
  {
    class non_recursive_filter_model : public QSortFilterProxyModel
    {
    public:
      non_recursive_filter_model (QObject* parent = NULL);

    protected:
      virtual bool filterAcceptsRow (int source_row, const QModelIndex &source_parent) const;

    private:
      bool is_shown (const QModelIndex& index) const;
    };
  }
}

#endif
