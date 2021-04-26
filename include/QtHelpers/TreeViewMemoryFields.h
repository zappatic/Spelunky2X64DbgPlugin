#pragma once

#include "Data/EntityDB.h"
#include "QtHelpers/HTMLDelegate.h"
#include "Spelunky2.h"
#include <QStandardItem>
#include <QTreeView>
#include <memory>
#include <unordered_map>

class TreeViewMemoryFields : public QTreeView
{
    Q_OBJECT
  public:
    TreeViewMemoryFields(EntityDB* entityDB, QWidget* parent = nullptr);
    void addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, QStandardItem* parent);

    void addRectMemoryFields(const std::string& rectName, QStandardItem* parent = nullptr);
    void addEntityDBMemoryFields(QStandardItem* parent = nullptr);

    QStandardItem* lookupTreeViewItem(const std::string& fieldName, uint8_t column, QStandardItem* parent);
    void updateValueForField(const MemoryField& field, const std::string& fieldNameOverride, const std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent = nullptr);

  private slots:
    void cellClicked(const QModelIndex& index);

  private:
    QStandardItemModel* mModel;
    EntityDB* mEntityDB;
    std::unique_ptr<HTMLDelegate> mHTMLDelegate;

    void updateRectValues(const std::string& rectName, const std::unordered_map<std::string, size_t>& offsets, QStandardItem* parent);
    void updateTableHeader();
};
