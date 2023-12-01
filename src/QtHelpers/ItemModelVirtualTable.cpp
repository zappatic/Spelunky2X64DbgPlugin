#include "QtHelpers/ItemModelVirtualTable.h"
#include "Configuration.h"
#include "Data/State.h"
#include "Data/VirtualTableLookup.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"

S2Plugin::ItemModelVirtualTable::ItemModelVirtualTable(VirtualTableLookup* vtl, QObject* parent) : QAbstractItemModel(parent), mVirtualTableLookup(vtl) {}

Qt::ItemFlags S2Plugin::ItemModelVirtualTable::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant S2Plugin::ItemModelVirtualTable::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto& entry = mVirtualTableLookup->entryForOffset(index.row());
        switch (index.column())
        {
            case gsColTableOffset:
                return entry.offset;
            case gsColCodeAddress:
                if (entry.isValidAddress)
                {
                    return QString::asprintf("<font color='green'><u>0x%016llX</u></font>", entry.value);
                }
                else
                {
                    return QString::asprintf("<span style='text-decoration: line-through'>0x%016llX</span>", entry.value);
                }
            case gsColTableAddress:
                return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", mVirtualTableLookup->tableAddressForEntry(entry));
            case gsColSymbolName:
            {
                QStringList l;
                for (const auto& symbol : entry.symbols)
                {
                    l << QString::fromStdString(symbol);
                }
                return l.join(", ");
            }
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelVirtualTable::rowCount(const QModelIndex& parent) const
{
    return mVirtualTableLookup->count();
}

int S2Plugin::ItemModelVirtualTable::columnCount(const QModelIndex& parent) const
{
    return 4;
}

QModelIndex S2Plugin::ItemModelVirtualTable::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex S2Plugin::ItemModelVirtualTable::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

QVariant S2Plugin::ItemModelVirtualTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsColTableOffset:
                return "Table offset";
            case gsColCodeAddress:
                return "Code address";
            case gsColTableAddress:
                return "Table address";
            case gsColSymbolName:
                return "Symbol";
        }
    }
    return QVariant();
}

void S2Plugin::ItemModelVirtualTable::detectEntities(ViewToolbar* toolbar)
{
    auto state = toolbar->state();
    auto spel2 = Spelunky2::get();
    auto entityDB = toolbar->entityDB();
    auto vtl = toolbar->virtualTableLookup();

    auto processEntities = [&](size_t layerEntities, uint32_t count)
    {
        size_t maximum = (std::min)(count, 10000u);
        for (auto x = 0; x < maximum; ++x)
        {
            auto entityPtr = layerEntities + (x * sizeof(size_t));
            auto entity = Script::Memory::ReadQword(entityPtr);
            auto entityVTableOffset = Script::Memory::ReadQword(entity);

            auto entityUid = Script::Memory::ReadDword(entity + 56);
            auto entityName = spel2->getEntityName(entity, entityDB);
            vtl->setSymbolNameForOffsetAddress(entityVTableOffset, entityName);
        }
    };

    beginResetModel();
    auto layer0 = Script::Memory::ReadQword(state->offsetForField("layer0"));
    auto layer0Count = Script::Memory::ReadDword(layer0 + 28);
    auto layer0Entities = Script::Memory::ReadQword(layer0 + 8);
    processEntities(layer0Entities, layer0Count);

    auto layer1 = Script::Memory::ReadQword(state->offsetForField("layer1"));
    auto layer1Count = Script::Memory::ReadDword(layer1 + 28);
    auto layer1Entities = Script::Memory::ReadQword(layer1 + 8);
    processEntities(layer1Entities, layer1Count);
    endResetModel();
}

S2Plugin::SortFilterProxyModelVirtualTable::SortFilterProxyModelVirtualTable(VirtualTableLookup* vtl, QObject* parent) : QSortFilterProxyModel(parent), mVirtualTableLookup(vtl) {}

bool S2Plugin::SortFilterProxyModelVirtualTable::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    const auto& entry = mVirtualTableLookup->entryForOffset(sourceRow);

    // only do text filtering when symbolless entries are not shown
    // because we will just jump to the first match in ViewVirtualTable::filterTextChanged
    if (!mShowSymbollessEntries && !mFilterString.isEmpty() && entry.symbols.size() > 0)
    {
        bool found = false;
        for (const auto& symbol : entry.symbols)
        {
            if (QString::fromStdString(symbol).contains(mFilterString, Qt::CaseInsensitive))
            {
                found = true;
            }
        }
        if (!found)
        {
            return false;
        }
    }

    if (!mShowImportedSymbols && entry.isAutoSymbol)
    {
        return false;
    }

    if (!mShowNonAddressEntries && !entry.isValidAddress)
    {
        return false;
    }

    if (!mShowSymbollessEntries && entry.symbols.size() == 0)
    {
        return false;
    }

    return true;
}

void S2Plugin::SortFilterProxyModelVirtualTable::setShowImportedSymbols(bool b)
{
    mShowImportedSymbols = b;
    invalidateFilter();
}

void S2Plugin::SortFilterProxyModelVirtualTable::setShowNonAddressEntries(bool b)
{
    mShowNonAddressEntries = b;
    invalidateFilter();
}

void S2Plugin::SortFilterProxyModelVirtualTable::setShowSymbollessEntries(bool b)
{
    mShowSymbollessEntries = b;
    invalidateFilter();
}

void S2Plugin::SortFilterProxyModelVirtualTable::setFilterString(const QString& f)
{
    mFilterString = f;
    invalidateFilter();
}

bool S2Plugin::SortFilterProxyModelVirtualTable::symbollessEntriesShown() const noexcept
{
    return mShowSymbollessEntries;
}
