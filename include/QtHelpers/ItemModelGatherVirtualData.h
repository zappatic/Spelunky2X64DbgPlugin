#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    static const uint8_t gsColGatherID = 0;
    static const uint8_t gsColGatherName = 1;
    static const uint8_t gsColGatherVirtualTableOffset = 2;
    static const uint8_t gsColGatherCollision1Present = 3;
    static const uint8_t gsColGatherCollision2Present = 4;
    static const uint8_t gsColGatherOpenPresent = 5;
    static const uint8_t gsColGatherDamagePresent = 6;
    static const uint8_t gsColGatherKillPresent = 7;
    static const uint8_t gsColGatherDestroyPresent = 8;
    static const uint8_t gsColGatherStatemachinePresent = 9;

    struct GatheredDataEntry
    {
        size_t id;
        QString name;
        size_t virtualTableOffset;
        bool collision1Present;
        bool collision2Present;
        bool openPresent;
        bool damagePresent;
        bool killPresent;
        bool destroyPresent;
        bool statemachinePresent;
    };

    class ItemModelGatherVirtualData : public QAbstractItemModel
    {
        Q_OBJECT

      public:
        ItemModelGatherVirtualData(QObject* parent = nullptr);

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        void gatherEntities();
        void gatherExtraObjects();
        void gatherAvailableVirtuals();
        float completionPercentage() const;
        std::string dumpJSON() const;
        std::string dumpVirtTable() const;
        std::string dumpCppEnum() const;
        bool isEntryCompleted(size_t index) const;

      private:
        std::vector<GatheredDataEntry> mEntries;

        void parseJSON();
    };

    class SortFilterProxyModelGatherVirtualData : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelGatherVirtualData(QObject* parent = nullptr);

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        void setHideCompleted(bool b);

      private:
        bool mHideCompleted = false;
    };

} // namespace S2Plugin
