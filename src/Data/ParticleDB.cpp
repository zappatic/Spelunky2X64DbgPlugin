#include "Data/ParticleDB.h"
#include "Configuration.h"

uintptr_t S2Plugin::ParticleDB::offsetForIndex(uint32_t idx) const
{
    if (ptr == 0)
        return 0;

    auto getParticleDBSize = []()
    {
        auto& fields = Configuration::get()->typeFields(MemoryFieldType::ParticleDB);
        size_t size = 0;
        for (auto& field : fields)
        {
            size += field.get_size();
        }
        return size;
    };
    // [Known Issue]: Static value, have to restart programm for size to update
    static size_t particleDBRecordSize = getParticleDBSize();

    return ptr + idx * particleDBRecordSize;
}
