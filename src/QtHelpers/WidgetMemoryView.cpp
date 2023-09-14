#include "QtHelpers/WidgetMemoryView.h"
#include "pluginmain.h"
#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QToolTip>

static const uint32_t gsMarginHor = 10;
static const uint32_t gsMarginVer = 5;

S2Plugin::WidgetMemoryView::WidgetMemoryView(QWidget* parent) : QWidget(parent)
{
    mFont = QFont("Courier", 11);
    mTextAdvance = QFontMetrics(mFont).size(Qt::TextSingleLine, "00");
    mSpaceAdvance = QFontMetrics(mFont).size(Qt::TextSingleLine, " ").width();
    setMouseTracking(true);
}

void S2Plugin::WidgetMemoryView::paintEvent(QPaintEvent* event)
{
    if (mOffset != 0 && mSize != 0)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.setFont(mFont);

        mToolTipRects.clear();
        painter.setBrush(Qt::black);
        uint32_t x = gsMarginHor;
        uint32_t y = gsMarginVer + mTextAdvance.height();
        uint32_t index = 0;
        for (size_t opCounter = mOffset; opCounter < (mOffset + mSize); ++opCounter)
        {
            // paint highlighted fields
            painter.setPen(Qt::transparent);
            for (const auto& field : mHighlightedFields)
            {
                if (opCounter == field.offset)
                {
                    auto rect = QRect(x, y - mTextAdvance.height() + 5, field.size * mTextAdvance.width() + ((field.size - 1) * mSpaceAdvance), mTextAdvance.height() - 2);
                    painter.setBrush(field.color);
                    painter.drawRoundedRect(rect, 4.0, 4.0);
                    mToolTipRects.emplace_back(ToolTipRect{rect, field.tooltip});
                }
            }

            // paint hex values
            painter.setPen(QPen(Qt::SolidLine));
            auto str = QString("%1").arg(Script::Memory::ReadByte(opCounter), 2, 16, QChar('0'));
            painter.drawText(x, y, str);
            x += mTextAdvance.width() + mSpaceAdvance;
            index++;
            if (index % 16 == 0)
            {
                y += mTextAdvance.height();
                x = gsMarginHor;
            }
        }
    }
}

QSize S2Plugin::WidgetMemoryView::sizeHint() const
{
    return minimumSizeHint();
}

QSize S2Plugin::WidgetMemoryView::minimumSizeHint() const
{
    auto totalWidth = ((mTextAdvance.width() + mSpaceAdvance) * 16) + (gsMarginHor * 2) - mSpaceAdvance;
    auto totalHeight = (mTextAdvance.height() * static_cast<uint32_t>(std::ceil(static_cast<double>(mSize) / 16.))) + (gsMarginVer * 2) + mTextAdvance.height();
    return QSize(totalWidth, totalHeight);
}

void S2Plugin::WidgetMemoryView::setOffsetAndSize(size_t offset, size_t size)
{
    mOffset = offset;
    mSize = size;
    update();
    updateGeometry();
    adjustSize();
}

void S2Plugin::WidgetMemoryView::clearHighlights()
{
    mHighlightedFields.clear();
    mToolTipRects.clear();
    update();
}

void S2Plugin::WidgetMemoryView::addHighlightedField(const std::string& tooltip, size_t offset, size_t size, const QColor& color)
{
    mHighlightedFields.emplace_back(HighlightedField{tooltip, offset, size, color});
}

void S2Plugin::WidgetMemoryView::mouseMoveEvent(QMouseEvent* event)
{
    for (const auto& ttr : mToolTipRects)
    {
        auto pos = event->pos();
        if (ttr.rect.contains(pos))
        {
            QToolTip::showText(mapToGlobal(pos), QString::fromStdString(ttr.tooltip));
        }
    }
}
