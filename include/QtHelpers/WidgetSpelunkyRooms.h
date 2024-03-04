#pragma once

#include <QWidget>
#include <unordered_map>

namespace S2Plugin
{
    class ViewToolbar;
    struct ToolTipRect;

    class WidgetSpelunkyRooms : public QWidget
    {
        Q_OBJECT

      public:
        WidgetSpelunkyRooms(const std::string& fieldName, ViewToolbar* toolbar, QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void setOffset(size_t offset);
        void setIsMetaData();

      protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

      private:
        QString mFieldName;
        ViewToolbar* mToolbar;
        bool mIsMetaData = false;
        size_t mOffset{0};
        QFont mFont;
        QSize mTextAdvance;
        uint8_t mSpaceAdvance;
        std::vector<ToolTipRect> mToolTipRects;
    };

} // namespace S2Plugin
