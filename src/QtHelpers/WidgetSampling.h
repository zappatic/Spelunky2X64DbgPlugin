#pragma once

#include <QPaintEvent>
#include <QWidget>

namespace S2Plugin
{
    class WidgetSampling : public QWidget
    {
        Q_OBJECT
      public:
        explicit WidgetSampling(QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      protected:
        void paintEvent(QPaintEvent* event) override;
    };
} // namespace S2Plugin
