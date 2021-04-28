#pragma once

#include <QStyledItemDelegate>

namespace S2Plugin
{
    class HTMLDelegate : public QStyledItemDelegate
    {
      protected:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    };
} // namespace S2Plugin