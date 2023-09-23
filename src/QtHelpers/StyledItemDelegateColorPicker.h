#pragma once

#include <QStyledItemDelegate>

namespace S2Plugin
{
    class StyledItemDelegateColorPicker : public QStyledItemDelegate
    {
      protected:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    };
} // namespace S2Plugin
