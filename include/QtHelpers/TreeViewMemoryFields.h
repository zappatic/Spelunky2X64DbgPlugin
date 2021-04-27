#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/HTMLDelegate.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include <QStandardItem>
#include <QTreeView>
#include <array>
#include <memory>
#include <unordered_map>


class TreeViewMemoryFields : public QTreeView
{
    Q_OBJECT
  public:
    TreeViewMemoryFields(ViewToolbar* toolbar, QWidget* parent = nullptr);
    void addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent = nullptr);
    void clear();
    void updateTableHeader();

    void addEntityDBMemoryFields(QStandardItem* parent = nullptr);
    void addStateMemoryFields(QStandardItem* parent = nullptr);
    void addEntityFields(QStandardItem* parent = nullptr);

    QStandardItem* lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent);
    void updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, const std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent = nullptr);

  private slots:
    void cellClicked(const QModelIndex& index);

  private:
    QStandardItemModel* mModel;
    ViewToolbar* mToolbar;
    std::unique_ptr<HTMLDelegate> mHTMLDelegate;
    std::array<uint32_t, 5> mSavedColumnWidths = {0};
};
