#include "QtHelpers/HTMLDelegate.h"
#include <QPainter>
#include <QTextDocument>

void S2Plugin::HTMLDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    if (mCenterVertically)
    {
        doc.setTextWidth(options.rect.width());
        auto centerVOffset = (options.rect.height() - doc.size().height()) / 2.0;
        painter->translate(options.rect.left(), options.rect.top() + centerVOffset);
    }
    else
    {
        painter->translate(options.rect.left(), options.rect.top() - 2);
    }
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

QSize S2Plugin::HTMLDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    doc.setDocumentMargin(2);
    return QSize(doc.idealWidth(), doc.size().height());
}

void S2Plugin::HTMLDelegate::setCenterVertically(bool b)
{
    mCenterVertically = b;
}
