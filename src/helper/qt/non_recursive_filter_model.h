// non_recursive_filter_model.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#pragma once

#include <QSortFilterProxyModel>

namespace helper
{
  namespace qt
  {
    class non_recursive_filter_model : public QSortFilterProxyModel
    {
    public:
      non_recursive_filter_model (QObject* parent = nullptr);

    protected:
      virtual bool filterAcceptsRow (int source_row, const QModelIndex &source_parent) const;

    private:
      bool is_shown (const QModelIndex& index) const;
    };
  }
}
