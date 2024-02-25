#include "Data/StringsTable.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <QString>

QString S2Plugin::StringsTable::stringForIndex(uint32_t id) const
{
    if (count() < id)
    {
        return QString("INVALID OR NOT APPLICABLE");
    }
    auto str = ReadConstBasicString<ushort>(Script::Memory::ReadQword(offsetForIndex(id)));
    return QString::fromUtf16(str.c_str(), str.size());
}

uintptr_t S2Plugin::StringsTable::offsetForIndex(size_t idx) const
{
    return ptr + idx * sizeof(uintptr_t);
}
