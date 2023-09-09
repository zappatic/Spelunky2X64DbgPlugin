#include "QtHelpers/ItemModelLoggerFields.h"
#include "Data/Logger.h"
#include "QtHelpers/TableViewLogger.h"

S2Plugin::ItemModelLoggerFields::ItemModelLoggerFields(Logger* logger, QObject* parent) : QAbstractItemModel(parent), mLogger(logger) {}

Qt::ItemFlags S2Plugin::ItemModelLoggerFields::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant S2Plugin::ItemModelLoggerFields::data(const QModelIndex& index, int role) const
{
    auto& field = mLogger->fieldAt(index.row());
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case gsLogFieldColColor:
            {
                return field.color;
            }
            case gsLogFieldColMemoryOffset:
            {
                return QString::asprintf("0x%016llX", field.memoryOffset);
            }
            case gsLogFieldColFieldName:
            {
                return QString::fromStdString(field.name);
            }
            case gsLogFieldColFieldType:
            {
                return QString::fromStdString(gsMemoryFieldTypeToStringMapping.at(field.type));
            }
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelLoggerFields::rowCount(const QModelIndex& parent) const
{
    return mLogger->fieldCount();
}

int S2Plugin::ItemModelLoggerFields::columnCount(const QModelIndex& parent) const
{
    return 4;
}

QModelIndex S2Plugin::ItemModelLoggerFields::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex S2Plugin::ItemModelLoggerFields::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

QVariant S2Plugin::ItemModelLoggerFields::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsLogFieldColColor:
                return "Color";
            case gsLogFieldColMemoryOffset:
                return "Memory offset";
            case gsLogFieldColFieldType:
                return "Type";
            case gsLogFieldColFieldName:
                return "Name";
        }
    }
    return QVariant();
}

void S2Plugin::ItemModelLoggerFields::removeRow(size_t index)
{
    beginRemoveRows(QModelIndex(), index, index);
}

void S2Plugin::ItemModelLoggerFields::removeRowEnd()
{
    endRemoveRows();
}

void S2Plugin::ItemModelLoggerFields::appendRow()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
}

void S2Plugin::ItemModelLoggerFields::appendRowEnd()
{
    endInsertRows();
}
