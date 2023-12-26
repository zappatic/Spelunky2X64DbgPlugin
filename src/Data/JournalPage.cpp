#include "Data/JournalPage.h"
#include "Configuration.h"
#include "Spelunky2.h"
#include "pluginmain.h"

S2Plugin::JournalPage::JournalPage(size_t offset, const std::string& pageType) : mJournalPagePtr(offset), mJournalPageType(pageType) {}

std::unordered_map<std::string, size_t>& S2Plugin::JournalPage::offsets()
{
    return mMemoryOffsets;
}

void S2Plugin::JournalPage::refreshOffsets()
{
    mMemoryOffsets.clear();
    auto offset = mJournalPagePtr;
    auto config = Configuration::get();
    for (const auto& field : config->typeFieldsOfInlineStruct(mJournalPageType))
    {
        offset = config->setOffsetForField(field, mJournalPageType + "." + field.name, offset, mMemoryOffsets);
    }
}

size_t S2Plugin::JournalPage::offsetForField(const std::string& fieldName) const
{
    auto full = mJournalPageType + "." + fieldName;
    if (mMemoryOffsets.count(full) == 0)
    {
        return 0;
    }
    return mMemoryOffsets.at(full);
}

void S2Plugin::JournalPage::interpretAs(const std::string& classType)
{
    mJournalPageType = classType;
}
