#include "QtHelpers/DatabaseHelper.h"

#include "Configuration.h"
#include "pluginmain.h"
#include <QString>

struct ComparisonField
{
    S2Plugin::MemoryFieldType type{S2Plugin::MemoryFieldType::None};
    size_t offset;
    uint8_t flag_index;
};
Q_DECLARE_METATYPE(ComparisonField)

size_t S2Plugin::DB::populateComparisonCombobox(QComboBox* CompareFieldComboBox, const std::vector<MemoryField>& fields, size_t offset, std::string prefix)
{
    for (const auto& field : fields)
    {
        if (field.isPointer)
        {
            // we don't do pointers as i don't think there are any important ones for comparison in databases

            offset += sizeof(uintptr_t);
            continue;
        }
        switch (field.type)
        {
            case MemoryFieldType::Skip:
                break;
            case MemoryFieldType::Flags32:
            case MemoryFieldType::Flags16:
            case MemoryFieldType::Flags8:
            {
                ComparisonField parrentFlag;
                parrentFlag.type = field.type;
                parrentFlag.offset = offset;
                CompareFieldComboBox->addItem(QString::fromStdString(prefix + field.name), QVariant::fromValue(parrentFlag));
                uint8_t flagCount = (field.type == MemoryFieldType::Flags16 ? 16 : (field.type == MemoryFieldType::Flags8 ? 8 : 32));
                for (uint8_t x = 1; x <= flagCount; ++x)
                {
                    ComparisonField flag;
                    flag.type = MemoryFieldType::Flag;
                    flag.offset = offset;
                    flag.flag_index = x - 1;
                    CompareFieldComboBox->addItem(QString::fromStdString(prefix + field.name + ".flag_" + std::to_string(x)), QVariant::fromValue(flag));
                }
                break;
            }
            case MemoryFieldType::DefaultStructType:
            {
                offset = populateComparisonCombobox(CompareFieldComboBox, Configuration::get()->typeFieldsOfDefaultStruct(field.jsonName), offset, prefix + field.name + ".");
                continue;
            }
            default:
            {
                ComparisonField tmp;
                tmp.offset = offset;
                tmp.type = field.type;
                CompareFieldComboBox->addItem(QString::fromStdString(prefix + field.name), QVariant::fromValue(tmp));
                break;
            }
        }
        offset += field.get_size();
    }
    return offset;
}

std::pair<QString, QVariant> S2Plugin::DB::valueForField(const QVariant& data, uintptr_t addr)
{
    ComparisonField compData = qvariant_cast<ComparisonField>(data);

    auto offset = addr + compData.offset;
    switch (compData.type)
    {
        // we only handle values that are present in the db's
        case MemoryFieldType::Byte:
        case MemoryFieldType::State8:
        {
            int8_t value = Script::Memory::ReadByte(offset);
            return std::make_pair(QString::asprintf("%d", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::CharacterDBID:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Flags8:
        {
            uint8_t value = Script::Memory::ReadByte(offset);
            return std::make_pair(QString::asprintf("%u", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Word:
        case MemoryFieldType::State16:
        {
            int16_t value = Script::Memory::ReadWord(offset);
            return std::make_pair(QString::asprintf("%d", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        {
            uint16_t value = Script::Memory::ReadWord(offset);
            return std::make_pair(QString::asprintf("%u", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::Dword:
        case MemoryFieldType::State32:
        {
            int32_t value = Script::Memory::ReadDword(offset);
            return std::make_pair(QString::asprintf("%ld", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::StringsTableID:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Flags32:
        {
            uint32_t value = Script::Memory::ReadDword(offset);
            return std::make_pair(QString::asprintf("%lu", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Qword:
        {
            int64_t value = Script::Memory::ReadQword(offset);
            return std::make_pair(QString::asprintf("%lld", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t value = Script::Memory::ReadQword(offset);
            return std::make_pair(QString::asprintf("%llu", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Float:
        {
            uint32_t dword = Script::Memory::ReadDword(offset);
            float value = reinterpret_cast<float&>(dword);
            return std::make_pair(QString::asprintf("%f", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Double:
        {
            size_t qword = Script::Memory::ReadQword(offset);
            double value = reinterpret_cast<double&>(qword);
            return std::make_pair(QString::asprintf("%lf", value), QVariant::fromValue(value));
        }
        case MemoryFieldType::Bool:
        {
            auto b = Script::Memory::ReadByte(offset);
            bool value = reinterpret_cast<bool&>(b);
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(b));
        }
        case MemoryFieldType::Flag:
        {
            uint8_t flagToCheck = compData.flag_index;
            bool isFlagSet = false;
            if (flagToCheck > 15)
                isFlagSet = ((Script::Memory::ReadDword(offset) & (1 << flagToCheck)) != 0);
            else if (flagToCheck > 7)
                isFlagSet = ((Script::Memory::ReadWord(offset) & (1 << flagToCheck)) != 0);
            else
                isFlagSet = ((Script::Memory::ReadByte(offset) & (1 << flagToCheck)) != 0);

            bool value = reinterpret_cast<bool&>(isFlagSet);
            return std::make_pair(value ? "True" : "False", QVariant::fromValue(isFlagSet));
        }
    }
    return std::make_pair("unknown", 0);
}
