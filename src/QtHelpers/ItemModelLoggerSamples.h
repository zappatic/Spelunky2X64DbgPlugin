#pragma once

#include "Spelunky2.h"
#include <QAbstractItemModel>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    class Logger;

    class ItemModelLoggerSamples : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        ItemModelLoggerSamples(Logger* logger, QObject* parent = nullptr);

        void reset();

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      private:
        Logger* mLogger;
    };
} // namespace S2Plugin
