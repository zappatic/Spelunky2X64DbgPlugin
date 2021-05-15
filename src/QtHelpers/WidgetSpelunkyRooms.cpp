#include "QtHelpers/WidgetSpelunkyRooms.h"
#include "pluginmain.h"
#include <QEvent>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QToolTip>
#include <array>

static const uint32_t gsMarginHor = 10;
static const uint32_t gsMarginVer = 5;
static constexpr size_t gsBufferSize = 8 * 16 * 2;     // 8x16 rooms * 2 bytes per room
static constexpr size_t gsHalfBufferSize = 8 * 16 * 1; // 8x16 rooms * 1 byte/bool per room

S2Plugin::WidgetSpelunkyRooms::WidgetSpelunkyRooms(const std::string& fieldName, ViewToolbar* toolbar, QWidget* parent)
    : QWidget(parent), mFieldName(QString::fromStdString(fieldName)), mToolbar(toolbar)
{
    mFont = QFont("Courier", 11);
    mTextAdvance = QFontMetrics(mFont).size(Qt::TextSingleLine, "00");
    mSpaceAdvance = QFontMetrics(mFont).size(Qt::TextSingleLine, " ").width();
    setMouseTracking(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed));
}

void S2Plugin::WidgetSpelunkyRooms::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setFont(mFont);

    auto rect = QRectF(QPointF(0, 0), sizeHint());
    rect.adjust(0, 0, -0.5, -0.5);
    painter.setBrush(Qt::white);
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.drawRect(rect);

    mToolTipRects.clear();
    painter.setBrush(Qt::black);
    uint32_t x = gsMarginHor;
    uint32_t y = gsMarginVer + mTextAdvance.height();

    painter.drawText(x, y, mFieldName);
    y += mTextAdvance.height() + gsMarginVer;
    if (mOffset != 0)
    {
        auto bufferSize = mIsMetaData ? gsHalfBufferSize : gsBufferSize;
        auto buffer = std::array<uint8_t, gsBufferSize>();
        Script::Memory::Read(mOffset, buffer.data(), bufferSize, nullptr);

        RoomCode currentRoomCode;
        uint32_t index = 0;
        for (auto counter = 0; counter < bufferSize; ++counter)
        {
            if (mIsMetaData)
            {
                if (buffer.at(counter) == 1)
                {
                    painter.setPen(QPen(Qt::white, 1));
                    painter.setBrush(Qt::black);
                    auto rect = QRect(x, y - mTextAdvance.height() + 5, mTextAdvance.width(), mTextAdvance.height() - 2);
                    painter.drawRect(rect);
                }
                else
                {
                    painter.setPen(QPen(Qt::black, 1));
                }
            }
            else
            {
                if (counter % 2 == 0)
                {
                    currentRoomCode = mToolbar->levelGen()->roomCodeForID(buffer.at(counter));
                    painter.setPen(Qt::transparent);
                    auto rect = QRect(x, y - mTextAdvance.height() + 5, 2 * mTextAdvance.width() + mSpaceAdvance, mTextAdvance.height() - 2);
                    painter.setBrush(currentRoomCode.color);
                    painter.drawRoundedRect(rect, 4.0, 4.0);
                    mToolTipRects.emplace_back(ToolTipRect{rect, currentRoomCode.name});
                }

                if (currentRoomCode.id == 0 || currentRoomCode.id == 9)
                {
                    painter.setPen(QPen(Qt::lightGray, 1));
                }
                else
                {
                    painter.setPen(QPen(Qt::black, 1));
                }
            }

            auto str = QString("%1").arg(buffer.at(counter), 2, 16, QChar('0'));
            painter.drawText(x, y, str);
            x += mTextAdvance.width() + mSpaceAdvance;

            auto cutoff = 16;
            if (mIsMetaData)
            {
                cutoff = 8;
            }

            if ((counter + 1) % cutoff == 0)
            {
                y += mTextAdvance.height();
                x = gsMarginHor;
            }
        }

        // draw level dimensions
        auto levelWidth = Script::Memory::ReadDword(mToolbar->state()->offsetForField("level_width_rooms"));
        auto levelHeight = Script::Memory::ReadDword(mToolbar->state()->offsetForField("level_height_rooms"));
        uint32_t borderX = gsMarginHor;
        uint32_t borderY = (2 * gsMarginVer) + mTextAdvance.height() + 4;
        uint32_t borderWidth, borderHeight;
        if (mIsMetaData)
        {
            borderWidth = (levelWidth * (mTextAdvance.width() + mSpaceAdvance)) - mSpaceAdvance;
            borderHeight = levelHeight * mTextAdvance.height();
        }
        else
        {
            borderWidth = (levelWidth * (2 * (mTextAdvance.width() + mSpaceAdvance))) - mSpaceAdvance;
            borderHeight = levelHeight * mTextAdvance.height();
        }
        auto border = QRect(borderX, borderY, borderWidth, borderHeight);
        border.adjust(-2, -2, +2, +2);
        painter.setPen(QPen(Qt::blue, 1));
        painter.setBrush(Qt::transparent);
        painter.drawRect(border);
    }
}

QSize S2Plugin::WidgetSpelunkyRooms::sizeHint() const
{
    return minimumSizeHint();
}

QSize S2Plugin::WidgetSpelunkyRooms::minimumSizeHint() const
{
    auto bufferSize = mIsMetaData ? gsHalfBufferSize : gsBufferSize;
    auto cutoff = 16;
    if (mIsMetaData)
    {
        cutoff = 8;
    }

    auto totalWidth = ((mTextAdvance.width() + mSpaceAdvance) * cutoff) + (gsMarginHor * 2) - mSpaceAdvance;
    auto totalHeight = gsMarginVer + mTextAdvance.height() + (mTextAdvance.height() * static_cast<uint32_t>(std::ceil(static_cast<double>(bufferSize) / static_cast<double>(cutoff)))) +
                       (gsMarginVer * 2) + mTextAdvance.height();
    return QSize(totalWidth, totalHeight);
}

void S2Plugin::WidgetSpelunkyRooms::setOffset(size_t offset)
{
    mOffset = offset;
    update();
    updateGeometry();
    adjustSize();
}

void S2Plugin::WidgetSpelunkyRooms::mouseMoveEvent(QMouseEvent* event)
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

void S2Plugin::WidgetSpelunkyRooms::setIsMetaData()
{
    mIsMetaData = true;
}
