#pragma once

#include "Data/Logger.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

namespace S2Plugin
{
    class WidgetSamplesPlot : public QWidget
    {
        Q_OBJECT
      public:
        WidgetSamplesPlot(Logger* logger, QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void leaveEvent(QEvent* event) override;

      private:
        Logger* mLogger;
        QPoint mCurrentMousePos = QPoint();
    };
} // namespace S2Plugin
