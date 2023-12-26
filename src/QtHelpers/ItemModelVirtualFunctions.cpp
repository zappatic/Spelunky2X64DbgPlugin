#include "QtHelpers/ItemModelVirtualFunctions.h"
#include "Configuration.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"

S2Plugin::ItemModelVirtualFunctions::ItemModelVirtualFunctions(const std::string& typeName, size_t offset, ViewToolbar* toolbar, QObject* parent)
    : QAbstractItemModel(parent), mTypeName(typeName), mMemoryOffset(offset), mToolbar(toolbar)
{
}

Qt::ItemFlags S2Plugin::ItemModelVirtualFunctions::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant S2Plugin::ItemModelVirtualFunctions::data(const QModelIndex& index, int role) const
{
    const VirtualFunction entry = Configuration::get()->virtualFunctionsOfType(mTypeName).at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
        {
            switch (index.column())
            {
                case gsColFunctionIndex:
                    return entry.index;
                case gsColFunctionSignature:
                    return QString("%1 %2::<b>%3</b>(%4)")
                        .arg(QString::fromStdString(entry.returnValue))
                        .arg(QString::fromStdString(entry.type))
                        .arg(QString::fromStdString(entry.name))
                        .arg(QString::fromStdString(entry.params));
                case gsColFunctionTableAddress:
                    return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", mMemoryOffset + (entry.index * 8));
                case gsColFunctionFunctionAddress:
                    return QString::asprintf("<font color='green'><u>0x%016llX</u></font>", Script::Memory::ReadQword(mMemoryOffset + (entry.index * 8)));
            }
            break;
        }
        case gsRoleFunctionIndex:
        {
            return entry.index;
        }
        case gsRoleFunctionTableAddress:
        {
            return mMemoryOffset + (entry.index * 8);
        }
        case gsRoleFunctionFunctionAddress:
        {
            return Script::Memory::ReadQword(mMemoryOffset + (entry.index * 8));
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelVirtualFunctions::rowCount(const QModelIndex& parent) const
{
    return Configuration::get()->virtualFunctionsOfType(mTypeName).size();
}

int S2Plugin::ItemModelVirtualFunctions::columnCount(const QModelIndex& parent) const
{
    return 4;
}

QModelIndex S2Plugin::ItemModelVirtualFunctions::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex S2Plugin::ItemModelVirtualFunctions::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

QVariant S2Plugin::ItemModelVirtualFunctions::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsColFunctionIndex:
                return "Index";
            case gsColFunctionSignature:
                return "Signature";
            case gsColFunctionTableAddress:
                return "Table Address";
            case gsColFunctionFunctionAddress:
                return "Function Address";
        }
    }
    return QVariant();
}

S2Plugin::SortFilterProxyModelVirtualFunctions::SortFilterProxyModelVirtualFunctions(const std::string& typeName, ViewToolbar* toolbar, QObject* parent)
    : QSortFilterProxyModel(parent), mTypeName(typeName), mToolbar(toolbar)
{
    setSortRole(gsRoleFunctionIndex);
}

bool S2Plugin::SortFilterProxyModelVirtualFunctions::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    auto leftValue = sourceModel()->data(left, gsRoleFunctionIndex).toLongLong();
    auto rightValue = sourceModel()->data(right, gsRoleFunctionIndex).toLongLong();
    return leftValue < rightValue;
}
