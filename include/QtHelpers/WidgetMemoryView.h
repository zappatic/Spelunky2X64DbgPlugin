#pragma once

#include <QStyleOptionTabWidgetFrame>
#include <QWidget>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    struct HighlightedField
    {
        std::string tooltip;
        size_t offset;
        size_t size;
        QColor color;
    };

    struct ToolTipRect
    {
        QRect rect;
        std::string tooltip;
    };

    class WidgetMemoryView : public QWidget
    {
        Q_OBJECT

      public:
        WidgetMemoryView(QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void setOffsetAndSize(size_t offset, size_t size);

        void clearHighlights();
        void addHighlightedField(const std::string& tooltip, size_t offset, size_t size, const QColor& color);

      protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

      private:
        size_t mOffset = 0;
        size_t mSize = 0;
        QFont mFont;
        QSize mTextAdvance;
        uint8_t mSpaceAdvance;

        std::vector<HighlightedField> mHighlightedFields;
        std::vector<ToolTipRect> mToolTipRects;
    };
} // namespace S2Plugin
