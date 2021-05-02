#include "QtHelpers/WidgetSpelunkyLevel.h"
#include "pluginmain.h"
#include <QPainter>

S2Plugin::WidgetSpelunkyLevel::WidgetSpelunkyLevel(QWidget* parent) : QWidget(parent) {}

void S2Plugin::WidgetSpelunkyLevel::paintEvent(QPaintEvent* event)
{
    auto painter = QPainter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    // DRAW ENTITY BLOCKS
    painter.save();
    painter.scale(msScaleFactor, msScaleFactor);
    painter.setPen(Qt::transparent);
    for (uint32_t i = 0; i < (std::min)(10000u, mEntitiesCount); ++i)
    {
        QColor colorToUse;
        auto entityPtr = mEntitiesOffset + (i * sizeof(size_t));
        auto entity = Script::Memory::ReadQword(entityPtr);
        auto entityDB = Script::Memory::ReadQword(entity + 8);
        auto entityUID = Script::Memory::ReadDword(entity + 56);
        auto entityType = Script::Memory::ReadDword(entityDB + 20);
        auto entityMask = Script::Memory::ReadDword(entityDB + 24);
        auto entityOverlay = Script::Memory::ReadQword(entity + 16);

        auto foundInIDs = (mEntityIDsToPaint.count(entityType) == 1);
        auto foundInUIDs = (mEntityUIDsToPaint.count(entityUID) == 1);
        auto foundInMasks = false;
        if (!foundInIDs && !foundInUIDs) // only check masks if not found elsewhere
        {
            for (const auto& [mask, color] : mEntityMasksToPaint)
            {
                if ((entityMask & mask) == mask)
                {
                    foundInMasks = true;
                    colorToUse = color;
                    break;
                }
            }
        }

        if (foundInIDs || foundInUIDs || foundInMasks)
        {
            auto [entityX, entityY] = getEntityCoordinates(entity);
            if (foundInIDs && !foundInUIDs)
            {
                colorToUse = mEntityIDsToPaint.at(entityType);
            }
            else if (foundInUIDs)
            {
                colorToUse = mEntityUIDsToPaint.at(entityUID);
            }
            painter.setBrush(colorToUse);
            painter.drawRect(QRectF(msMarginHor + entityX, msMarginVer + msLevelMaxHeight - entityY, 1.0, 1.0));
        }
    }
    painter.restore();

    // DRAW BORDER
    painter.setPen(QPen(Qt::black, 1.0));
    painter.setBrush(Qt::transparent);
    painter.drawRect(QRectF(msMarginHor * msScaleFactor, msMarginVer * msScaleFactor, ((msLevelMaxWidth + msMarginHor) * msScaleFactor) - .5, ((msLevelMaxHeight + msMarginVer) * msScaleFactor) - .5));
}

void S2Plugin::WidgetSpelunkyLevel::loadEntities(size_t entitiesOffset, uint32_t entitiesCount)
{
    mEntitiesOffset = entitiesOffset;
    mEntitiesCount = entitiesCount;
    update();
}

void S2Plugin::WidgetSpelunkyLevel::paintEntityID(uint32_t entityTypeID, const QColor& color)
{
    mEntityIDsToPaint[entityTypeID] = color;
}

void S2Plugin::WidgetSpelunkyLevel::paintEntityUID(uint32_t entityUID, const QColor& color)
{
    mEntityUIDsToPaint[entityUID] = color;
}

void S2Plugin::WidgetSpelunkyLevel::paintEntityMask(uint32_t entityMask, const QColor& color)
{
    mEntityMasksToPaint[entityMask] = color;
}

void S2Plugin::WidgetSpelunkyLevel::clearPaintedEntities()
{
    mEntityIDsToPaint.clear();
    mEntityUIDsToPaint.clear();
    update();
}

QSize S2Plugin::WidgetSpelunkyLevel::minimumSizeHint() const
{
    auto width = msScaleFactor * ((msMarginHor * 2) + msLevelMaxWidth);
    auto height = msScaleFactor * ((msMarginVer * 2) + msLevelMaxHeight);
    return QSize(width, height);
}

QSize S2Plugin::WidgetSpelunkyLevel::sizeHint() const
{
    return minimumSizeHint();
}

std::pair<float, float> S2Plugin::WidgetSpelunkyLevel::getEntityCoordinates(size_t entityOffset) const
{
    auto rawX = Script::Memory::ReadDword(entityOffset + 64);
    auto rawY = Script::Memory::ReadDword(entityOffset + 68);
    float entityX = reinterpret_cast<float&>(rawX);
    float entityY = reinterpret_cast<float&>(rawY);

    auto entityOverlay = Script::Memory::ReadQword(entityOffset + 16);
    if (entityOverlay != 0)
    {
        auto [addX, addY] = getEntityCoordinates(entityOverlay);
        entityX += addX;
        entityY += addY;
    }
    return std::make_pair(entityX, entityY);
}