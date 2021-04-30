#pragma once

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "Data/MemoryMappedData.h"
#include "Data/State.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetMemoryView.h"
#include "Spelunky2.h"
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class Entity : MemoryMappedData
    {
      public:
        Entity(size_t offset, TreeViewMemoryFields* tree, WidgetMemoryView* memoryView, EntityDB* entityDB, Configuration* config);

        void refreshOffsets();
        void refreshValues();
        void interpretAs(MemoryFieldType classType);
        std::deque<MemoryFieldType> classHierarchy() const;
        void populateTreeView();
        void populateMemoryView();

        size_t totalMemorySize() const noexcept;
        size_t memoryOffset() const noexcept;

        static size_t findEntityByUID(uint32_t uid, State* state);

      private:
        size_t mEntityPtr = 0;
        TreeViewMemoryFields* mTree;
        WidgetMemoryView* mMemoryView;
        MemoryFieldType mEntityType = MemoryFieldType::ClassEntity;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
        std::unordered_map<MemoryFieldType, QStandardItem*> mTreeViewSectionItems;

        size_t mTotalMemorySize = 0;
        void highlightField(MemoryField field, const std::string& fieldNameOverride, const QColor& color);
    };
} // namespace S2Plugin