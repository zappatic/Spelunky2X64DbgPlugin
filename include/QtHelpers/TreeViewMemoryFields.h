#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/HTMLDelegate.h"
#include "Spelunky2.h"
#include "ViewToolbar.h"
#include <QStandardItem>
#include <QTreeView>
#include <memory>
#include <unordered_map>

class TreeViewMemoryFields : public QTreeView
{
    Q_OBJECT
  public:
    TreeViewMemoryFields(EntityDB* entityDB, ViewToolbar* toolbar, QWidget* parent = nullptr);
    void addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent);

    void addEntityDBMemoryFields(QStandardItem* parent = nullptr);
    void addStateMemoryFields(QStandardItem* parent = nullptr);
    void addEntityFields(QStandardItem* parent = nullptr);

    QStandardItem* lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent);
    void updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, const std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent = nullptr);

  private slots:
    void cellClicked(const QModelIndex& index);

  private:
    QStandardItemModel* mModel;
    EntityDB* mEntityDB;
    ViewToolbar* mToolbar;
    std::unique_ptr<HTMLDelegate> mHTMLDelegate;

    void updateTableHeader();
};
