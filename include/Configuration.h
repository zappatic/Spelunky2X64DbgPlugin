#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    struct MemoryField;
    struct VirtualFunction;
    struct Spelunky2;
    enum class MemoryFieldType;

    class Configuration
    {
      public:
        Configuration();

        void load();
        bool isValid() const noexcept;
        std::string lastError() const noexcept;

        const std::unordered_map<std::string, std::string>& entityClassHierarchy() const noexcept;
        const std::vector<std::pair<std::string, std::string>>& defaultEntityClassTypes() const noexcept;
        std::vector<std::string> classHierarchyOfEntity(const std::string& entityName) const;

        const std::vector<MemoryField>& typeFields(const MemoryFieldType& type) const;
        const std::vector<MemoryField>& typeFieldsOfEntitySubclass(const std::string& type) const;
        const std::vector<MemoryField>& typeFieldsOfPointer(const std::string& type) const;
        const std::vector<MemoryField>& typeFieldsOfInlineStruct(const std::string& type) const;
        std::vector<VirtualFunction> virtualFunctionsOfType(const std::string& field) const;

        bool isEntitySubclass(const std::string& type) const;
        bool isPointer(const std::string& type) const;
        bool isInlineStruct(const std::string& type) const;
        bool isBuiltInType(const std::string& type) const;
        int getAlingment(const std::string& type) const;

        std::string flagTitle(const std::string& fieldName, uint8_t flagNumber);
        std::string stateTitle(const std::string& fieldName, int64_t state);
        const std::unordered_map<int64_t, std::string>& stateTitlesOfField(const std::string& fieldName);

        Spelunky2* spelunky2() const noexcept;

      private:
        bool mIsValid = false;
        std::string mErrorString;
        std::unique_ptr<Spelunky2> mSpelunky2;

        std::unordered_map<std::string, std::string> mEntityClassHierarchy;
        std::vector<std::pair<std::string, std::string>> mDefaultEntityClassTypes;
        std::unordered_map<MemoryFieldType, std::vector<MemoryField>> mTypeFields;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsEntitySubclasses;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsPointers;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsInlineStructs;
        std::unordered_map<std::string, std::unordered_map<uint8_t, std::string>> mFlagTitles;  // fieldname => (flagnr 1-based => title)
        std::unordered_map<std::string, std::unordered_map<int64_t, std::string>> mStateTitles; // fieldname => (state => title)
        std::unordered_map<std::string, std::vector<VirtualFunction>> mVirtualFunctions;
        std::unordered_map<std::string, uint8_t> mAlignments;

        void processJSON(const std::string& j);
        bool isKnownEntitySubclass(const std::string& typeName) const;
    };
} // namespace S2Plugin
