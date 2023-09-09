#pragma once

#include <QWidget>
#include <cstdint>
#include <unordered_map>

namespace S2Plugin
{
    class WidgetSpelunkyLevel : public QWidget
    {
        Q_OBJECT

      public:
        explicit WidgetSpelunkyLevel(QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void loadEntities(size_t entitiesOffset, uint32_t entitiesCount);
        void paintEntityID(uint32_t entityTypeID, const QColor& color);
        void paintEntityMask(uint32_t entityMask, const QColor& color);
        void paintEntityUID(uint32_t entityUID, const QColor& color);
        void clearPaintedEntities();
        void clearPaintedEntityUID(uint32_t entityUID);

      protected:
        void paintEvent(QPaintEvent* event) override;

      private:
        size_t mEntitiesOffset;
        uint32_t mEntitiesCount;

        std::unordered_map<uint32_t, QColor> mEntityMasksToPaint;
        std::unordered_map<uint32_t, QColor> mEntityIDsToPaint;
        std::unordered_map<uint32_t, QColor> mEntityUIDsToPaint;
        std::pair<float, float> getEntityCoordinates(size_t entityOffset) const;

        static constexpr float msLevelMaxHeight = 125.0;
        static constexpr float msLevelMaxWidth = 3. + 3. + (8 * 10);
        static constexpr uint8_t msMarginVer = 1;
        static constexpr uint8_t msMarginHor = 1;
        static constexpr float msScaleFactor = 5.;
    };

} // namespace S2Plugin
