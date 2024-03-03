#pragma once

#include "Spelunky2.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <QString>
#include <memory>

uintptr_t S2Plugin::Spelunky2::getAfterBundle(uintptr_t sectionStart, size_t sectionSize)
{
    constexpr size_t sevenMegs = 7ull * 1024 * 1024;
    auto Spelunky2AfterBundle = Script::Pattern::FindMem(sectionStart + sectionSize - sevenMegs, sevenMegs, "55 41 57 41 56 41 55 41 54");
    if (Spelunky2AfterBundle == 0)
        displayError("Lookup error: unable to find 'after_bundle' location");

    return Spelunky2AfterBundle;
}

uintptr_t S2Plugin::Spelunky2::get_GameManagerPtr()
{
    if (mGameManagerPtr != 0)
        return mGameManagerPtr;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "C6 80 39 01 00 00 00 48");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find GameManager");
        return 0;
    }

    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 10);
    auto offsetPtr = instructionOffset + pcOffset + 14;
    mGameManagerPtr = Script::Memory::ReadQword(offsetPtr);
    return mGameManagerPtr;
}

const S2Plugin::EntityDB& S2Plugin::Spelunky2::get_EntityDB()
{
    if (mEntityDB.ptr != 0)
        return mEntityDB;

    auto instructionEntitiesPtr = Script::Pattern::FindMem(afterBundle, afterBundleSize, "A4 84 E4 CA DA BF 4E 83");
    if (instructionEntitiesPtr == 0)
    {
        displayError("Lookup error: unable to find EntityDB");
        return mEntityDB;
    }

    auto entitiesPtr = instructionEntitiesPtr - 33 + 7 + (duint)Script::Memory::ReadDword(instructionEntitiesPtr - 30);
    mEntityDB.ptr = Script::Memory::ReadQword(entitiesPtr);
    return mEntityDB;
}

const S2Plugin::TextureDB& S2Plugin::Spelunky2::get_TextureDB()
{
    if (mTextureDB.ptr != 0)
        return mTextureDB;

    auto instructionPtr = Script::Pattern::FindMem(afterBundle, afterBundleSize, "4C 89 C6 41 89 CF 8B 1D");
    if (instructionPtr == 0)
    {
        displayError("Lookup error: unable to find TextureDB");
        return mTextureDB;
    }

    auto textureStartAddress = instructionPtr + 12 + (duint)Script::Memory::ReadDword(instructionPtr + 8);
    auto textureCount = Script::Memory::ReadQword(textureStartAddress);
    mTextureDB.ptr = textureStartAddress + 0x8;

    constexpr uintptr_t textureSize = 0x40ull;
    for (auto x = 0; x < (std::min)(500ull, textureCount); ++x)
    {
        uintptr_t offset = mTextureDB.ptr + textureSize * x;
        auto textureID = Script::Memory::ReadQword(offset);

        auto nameOffset = offset + 0x8;

        size_t value = (nameOffset == 0 ? 0 : Script::Memory::ReadQword(Script::Memory::ReadQword(nameOffset)));
        if (value != 0)
        {
            std::string name = ReadConstString(value);

            mTextureDB.mTextureNamesStringList << QString("Texture %1 (%2)").arg(textureID).arg(QString::fromStdString(name));
            mTextureDB.mTextures.emplace(textureID, std::make_pair(std::move(name), offset));
        }
    }
    return mTextureDB;
}

uintptr_t S2Plugin::Spelunky2::get_OnlinePtr()
{
    if (mOnlinePtr != 0)
        return mOnlinePtr;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8B 05 ?? ?? ?? ?? 80 B8 00 02 00 00 FF");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find Online");
        return 0;
    }
    auto relativeOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mOnlinePtr = Script::Memory::ReadQword(instructionOffset + 7 + relativeOffset);

    return mOnlinePtr;
}

const S2Plugin::ParticleDB& S2Plugin::Spelunky2::get_ParticleDB()
{
    if (mParticleDB.ptr != 0)
        return mParticleDB;

    // Spelunky 1.20.4d, 1.23.1b: last id = 0xDB 219
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "FE FF FF FF 66 C7 05");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find ParticleDB");
        return mParticleDB;
    }
    mParticleDB.ptr = instructionOffset + 13 + (duint)Script::Memory::ReadDword(instructionOffset + 7);

    return mParticleDB;
}

const S2Plugin::CharacterDB& S2Plugin::Spelunky2::get_CharacterDB()
{
    if (mCharacterDB.ptr != 0)
        return mCharacterDB;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 6B C3 2C 48 8D 15 ?? ?? ?? ?? 48");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find CharacterDB");
        return mCharacterDB;
    }
    mCharacterDB.ptr = instructionOffset + 11 + (duint)Script::Memory::ReadDword(instructionOffset + 7);

    constexpr size_t characterSize = 0x2C;
    auto& stringsTable = get_StringsTable();
    for (size_t x = 0; x < 20; ++x)
    {
        size_t startOffset = mCharacterDB.ptr + (x * characterSize);
        size_t offset = startOffset;
        QString characterName = stringsTable.stringForIndex(Script::Memory::ReadDword(offset + 0x14));

        mCharacterDB.mCharacterNamesStringList << characterName;
    }
    return mCharacterDB;
}

const S2Plugin::StringsTable& S2Plugin::Spelunky2::get_StringsTable()
{
    if (mStringsTable.ptr != 0)
        return mStringsTable;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8D 15 ?? ?? ?? ?? 4C 8B 0C CA");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find StringsTable");
        return mStringsTable;
    }
    auto relativeOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mStringsTable.ptr = instructionOffset + 7 + relativeOffset;

    for (auto stringIndex = 0; stringIndex < 5000; ++stringIndex)
    {
        size_t stringPointer = Script::Memory::ReadQword(mStringsTable.ptr + (stringIndex * sizeof(uintptr_t)));
        if (Script::Memory::IsValidPtr(stringPointer))
        {
            mStringsTable.size++;
        }
        else
            break;
    }
    return mStringsTable;
}

const S2Plugin::VirtualTableLookup& S2Plugin::Spelunky2::get_VirtualTableLookup()
{
    if (mVirtualTableLookup.mTableStartAddress != 0)
        return mVirtualTableLookup;

    size_t gsAmountOfPointers = mVirtualTableLookup.count();
    mVirtualTableLookup.mOffsetToTableEntries.reserve(gsAmountOfPointers);

    // From 1.23.2 on, the base isn't on D3Dcompile any more, so just look up the first pointer by pattern
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8D 0D ?? ?? ?? ?? 48 89 0D ?? ?? ?? ?? 48 C7 05");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find VirtualTable start");
        return mVirtualTableLookup;
    }

    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mVirtualTableLookup.mTableStartAddress = instructionOffset + pcOffset + 7;

    // import the pointers
    auto buffer = std::make_unique<size_t[]>(gsAmountOfPointers);

    Script::Memory::Read(mVirtualTableLookup.mTableStartAddress, buffer.get(), gsAmountOfPointers * sizeof(size_t), nullptr);
    for (size_t x = 0; x < gsAmountOfPointers; ++x)
    {
        size_t pointer = buffer[x];
        VirtualTableEntry e;
        e.isValidAddress = Script::Memory::IsValidPtr(pointer);
        e.offset = x;
        e.value = pointer;
        mVirtualTableLookup.mOffsetToTableEntries.emplace(x, std::move(e));
    }
    return mVirtualTableLookup;
}
