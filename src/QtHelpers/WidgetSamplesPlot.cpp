#include "QtHelpers/WidgetSamplesPlot.h"
#include "Configuration.h"
#include "Data/Logger.h"
#include "pluginmain.h"
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QScrollArea>

static const uint8_t gsPlotMargin = 5;

S2Plugin::WidgetSamplesPlot::WidgetSamplesPlot(Logger* logger, QWidget* parent) : QWidget(parent), mLogger(logger)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
}

void S2Plugin::WidgetSamplesPlot::paintEvent(QPaintEvent* event)
{
    auto painter = QPainter(this);

    painter.save();

    painter.fillRect(rect(), Qt::black);
    painter.setPen(Qt::darkGray);
    auto paintBounds = rect().adjusted(0, 0, -1, -1);
    float drawHeight = paintBounds.height() - (2 * gsPlotMargin);
    painter.drawRect(paintBounds);
    painter.translate(gsPlotMargin, gsPlotMargin);

    for (auto i = 0; i < mLogger->fieldCount(); ++i)
    {
        const auto& field = mLogger->fieldAt(i);
        painter.setPen(field.color);

        auto [lowerBound, upperBound] = mLogger->sampleBounds(field);
        const auto& samples = mLogger->samplesForField(field.uuid);
        int x = 0;
        uint16_t mappedY = 0;
        uint16_t prevX = 0, prevY = 0;
        bool first = true;
        for (const auto& sample : samples)
        {
            switch (field.type)
            {
                case MemoryFieldType::Byte:
                {
                    mappedY = ((std::any_cast<int8_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::UnsignedByte:
                case MemoryFieldType::Bool:
                case MemoryFieldType::Flags8:
                case MemoryFieldType::State8:
                case MemoryFieldType::CharacterDBID:
                {
                    mappedY = ((std::any_cast<uint8_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::Word:
                {
                    mappedY = ((std::any_cast<int16_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::UnsignedWord:
                case MemoryFieldType::Flags16:
                case MemoryFieldType::State16:
                {
                    mappedY = ((std::any_cast<uint16_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::Dword:
                case MemoryFieldType::TextureDBID:
                case MemoryFieldType::EntityUID:
                case MemoryFieldType::State32:
                {
                    mappedY = ((std::any_cast<int32_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::UnsignedDword:
                case MemoryFieldType::Flags32:
                case MemoryFieldType::EntityDBID:
                case MemoryFieldType::ParticleDBID:
                case MemoryFieldType::StringsTableID:
                {
                    mappedY = ((std::any_cast<uint32_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::Float:
                {
                    mappedY = ((std::any_cast<float>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::Double:
                {
                    mappedY = ((std::any_cast<double>(sample) - lowerBound) / (double)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::Qword:
                {
                    mappedY = ((std::any_cast<int64_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
                case MemoryFieldType::UnsignedQword:
                {
                    mappedY = ((std::any_cast<uint64_t>(sample) - lowerBound) / (float)(upperBound - lowerBound)) * drawHeight;
                    break;
                }
            }
            if (!first)
            {
                painter.drawLine(prevX, prevY, x, drawHeight - mappedY);
            }
            first = false;
            prevX = x;
            prevY = drawHeight - mappedY;
            x++;
        }
    }
    painter.restore();

    if (!mCurrentMousePos.isNull() && underMouse())
    {
        painter.save();
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        static const auto font = QFont("Arial", 10);
        painter.setFont(font);

        painter.setPen(Qt::cyan);
        painter.drawLine(mCurrentMousePos.x(), 0, mCurrentMousePos.x(), paintBounds.height());

        int64_t sampleIndex = static_cast<int64_t>(mCurrentMousePos.x()) - gsPlotMargin;
        if (sampleIndex >= 0 && sampleIndex < static_cast<int64_t>(mLogger->sampleCount()))
        {
            auto scrollArea = qobject_cast<QScrollArea*>(parent()->parent());
            auto drawOnLeftSide = (scrollArea->mapFromGlobal(QCursor::pos()).x() > (scrollArea->width() / 2));

            QString sampleCaption = QString("Sample %1").arg(sampleIndex);
            if (drawOnLeftSide)
            {
                auto sampleCaptionSize = QFontMetrics(font).size(Qt::TextSingleLine, sampleCaption);
                painter.drawText(QPoint(mCurrentMousePos.x() - 15 - sampleCaptionSize.width(), mCurrentMousePos.y()), sampleCaption);
            }
            else
            {
                painter.drawText(QPoint(mCurrentMousePos.x() + 15, mCurrentMousePos.y()), sampleCaption);
            }

            uint16_t y = 15;
            for (auto i = 0; i < mLogger->fieldCount(); ++i)
            {
                const auto& field = mLogger->fieldAt(i);
                const auto& samples = mLogger->samplesForField(field.uuid);
                const auto& sample = samples.at(sampleIndex);
                QString caption;
                switch (field.type)
                {
                    case MemoryFieldType::Byte:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<int8_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::UnsignedByte:
                    case MemoryFieldType::Bool:
                    case MemoryFieldType::Flags8:
                    case MemoryFieldType::State8:
                    case MemoryFieldType::CharacterDBID:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<uint8_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::Word:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<int16_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::UnsignedWord:
                    case MemoryFieldType::Flags16:
                    case MemoryFieldType::State16:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<uint16_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::Dword:
                    case MemoryFieldType::TextureDBID:
                    case MemoryFieldType::EntityUID:
                    case MemoryFieldType::State32:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<int32_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::UnsignedDword:
                    case MemoryFieldType::Flags32:
                    case MemoryFieldType::EntityDBID:
                    case MemoryFieldType::ParticleDBID:
                    case MemoryFieldType::StringsTableID:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<uint32_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::Float:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<float>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::Double:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<double>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::Qword:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<int64_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                    case MemoryFieldType::UnsignedQword:
                    {
                        caption = QString("%1 (%2)").arg(std::any_cast<uint64_t>(sample)).arg(QString::fromStdString(field.name));
                        break;
                    }
                }
                painter.setPen(field.color);
                if (drawOnLeftSide)
                {
                    auto captionSize = QFontMetrics(font).size(Qt::TextSingleLine, caption);
                    painter.drawText(QPoint(mCurrentMousePos.x() - 15 - captionSize.width(), mCurrentMousePos.y() + y), caption);
                }
                else
                {
                    painter.drawText(QPoint(mCurrentMousePos.x() + 15, mCurrentMousePos.y() + y), caption);
                }
                y += 15;
            }
        }
        painter.restore();
    }
}

void S2Plugin::WidgetSamplesPlot::mouseMoveEvent(QMouseEvent* event)
{
    mCurrentMousePos = event->pos();
    update();
}

void S2Plugin::WidgetSamplesPlot::leaveEvent(QEvent* event)
{
    update();
}

QSize S2Plugin::WidgetSamplesPlot::minimumSizeHint() const
{
    return QSize(mLogger->sampleCount() + (gsPlotMargin * 2) + 100, 50);
}

QSize S2Plugin::WidgetSamplesPlot::sizeHint() const
{
    return minimumSizeHint();
}
