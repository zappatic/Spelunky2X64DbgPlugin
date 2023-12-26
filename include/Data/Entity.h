#pragma once

#include <QStandardItem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    struct EntityDB;
    struct TreeViewMemoryFields;
    struct WidgetMemoryView;
    struct State;
    struct MemoryField;

    class Entity
    {
      public:
        Entity(size_t offset, TreeViewMemoryFields* tree, WidgetMemoryView* memoryView, WidgetMemoryView* comparisonMemoryView, EntityDB* entityDB);

        void refreshOffsets();
        void refreshValues();
        void interpretAs(const std::string& classType);
        std::string entityType() const noexcept;
        std::vector<std::string> classHierarchy() const;
        void populateTreeView();
        void populateMemoryView();

        size_t totalMemorySize() const noexcept;
        size_t memoryOffset() const noexcept;
        uint32_t uid() const noexcept;
        uint32_t comparisonUid() const noexcept;
        uint8_t cameraLayer() const noexcept;
        uint8_t comparisonCameraLayer() const noexcept;
        void label() const;
        void compareToEntity(size_t comparisonOffset);
        size_t comparedEntityMemoryOffset() const noexcept;
        void updateComparedMemoryViewHighlights();

        static size_t findEntityByUID(uint32_t uid, State* state);

      private:
        size_t mEntityPtr = 0;
        size_t mComparisonEntityPtr = 0;
        TreeViewMemoryFields* mTree;
        WidgetMemoryView* mMemoryView;
        WidgetMemoryView* mComparisonMemoryView;
        std::string mEntityType = "Entity";
        std::string mEntityName;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
        std::unordered_map<std::string, QStandardItem*> mTreeViewSectionItems;

        size_t mTotalMemorySize = 0;
        void highlightField(MemoryField field, const std::string& fieldNameOverride, const QColor& color);
        void highlightComparisonField(MemoryField field, const std::string& fieldNameOverride);
    };
} // namespace S2Plugin
