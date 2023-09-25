#pragma once

#include <QColor>
#include <QString>
#include <QTimer>
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    struct ItemModelLoggerFields;
    enum class MemoryFieldType;

    struct LoggerField
    {
        size_t memoryOffset;
        std::string name;
        MemoryFieldType type;
        QColor color;
        std::string uuid;
    };

    class Logger : public QObject
    {
        Q_OBJECT
      public:
        explicit Logger(QObject* parent = nullptr);

        void setTableModel(ItemModelLoggerFields* tableModel);

        const LoggerField& fieldAt(size_t fieldIndex) const;
        size_t fieldCount() const noexcept;

        void addField(const LoggerField& field);
        void removeFieldAt(size_t fieldIndex);
        void updateFieldColor(size_t fieldIndex, const QColor& newColor);

        const std::vector<std::any>& samplesForField(const std::string& fieldUUID) const;
        size_t sampleCount() const noexcept;
        std::pair<int64_t, int64_t> sampleBounds(const LoggerField& field) const;

        void start(size_t samplePeriod, size_t duration);

      signals:
        void samplingEnded();
        void fieldsChanged();

      private slots:
        void sample();
        void durationEnded();

      private:
        std::vector<LoggerField> mFields;
        ItemModelLoggerFields* mTableModel = nullptr;

        std::unique_ptr<QTimer> mSampleTimer;
        std::unique_ptr<QTimer> mDurationTimer;
        std::unordered_map<std::string, std::vector<std::any>> mSamples; // field uuid -> value
    };
} // namespace S2Plugin
