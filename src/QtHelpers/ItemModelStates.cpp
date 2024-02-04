#include "QtHelpers/ItemModelStates.h"

S2Plugin::ItemModelStates::ItemModelStates(const std::vector<std::pair<int64_t, std::string>>& states, QObject* parent) : QAbstractItemModel(parent)
{
    mStates = states;
}

Qt::ItemFlags S2Plugin::ItemModelStates::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant S2Plugin::ItemModelStates::data(const QModelIndex& index, int role) const
{
    auto& [stateID, state] = mStates.at(index.row());
    if (role == Qt::DisplayRole)
    {
        return QString("%1: %2").arg(stateID).arg(QString::fromStdString(state));
    }
    else if (role == Qt::UserRole)
    {
        return stateID;
    }
    return QVariant();
}

int S2Plugin::ItemModelStates::rowCount(const QModelIndex& parent) const
{
    return mStates.size();
}

int S2Plugin::ItemModelStates::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QModelIndex S2Plugin::ItemModelStates::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex S2Plugin::ItemModelStates::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

S2Plugin::SortFilterProxyModelStates::SortFilterProxyModelStates(const std::vector<std::pair<int64_t, std::string>>& states, QObject* parent) : QSortFilterProxyModel(parent)
{
    mStates = states;
    setSortRole(Qt::UserRole);
}

bool S2Plugin::SortFilterProxyModelStates::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    auto leftValue = sourceModel()->data(left, Qt::UserRole).toLongLong();
    auto rightValue = sourceModel()->data(right, Qt::UserRole).toLongLong();
    return leftValue < rightValue;
}
