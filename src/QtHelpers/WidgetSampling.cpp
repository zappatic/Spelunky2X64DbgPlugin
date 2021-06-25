#include "QtHelpers/WidgetSampling.h"
#include <QPainter>

S2Plugin::WidgetSampling::WidgetSampling(QWidget* parent) : QWidget(parent) {}

void S2Plugin::WidgetSampling::paintEvent(QPaintEvent* event)
{
    auto painter = QPainter(this);
    painter.save();

    painter.fillRect(rect(), Qt::white);
    painter.setPen(Qt::darkGray);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    static const auto caption = QString("Sampling...");
    static const auto font = QFont("Arial", 16);
    static const auto captionSize = QFontMetrics(font).size(Qt::TextSingleLine, caption);

    painter.setFont(font);
    painter.setPen(Qt::lightGray);
    painter.drawText(QPointF((width() / 2.) - (captionSize.width() / 2.), (height() / 2.) - (captionSize.height() / 2.)), caption);

    painter.restore();
}

QSize S2Plugin::WidgetSampling::minimumSizeHint() const
{
    return QSize(400, 400);
}

QSize S2Plugin::WidgetSampling::sizeHint() const
{
    return minimumSizeHint();
}
