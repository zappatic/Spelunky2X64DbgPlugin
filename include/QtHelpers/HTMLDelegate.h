#pragma once

#include <QStyledItemDelegate>

namespace S2Plugin
{
    class HTMLDelegate : public QStyledItemDelegate
    {
      public:
        void setCenterVertically(bool b);

      protected:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

      private:
        bool mCenterVertically = false;
    };
} // namespace S2Plugin
