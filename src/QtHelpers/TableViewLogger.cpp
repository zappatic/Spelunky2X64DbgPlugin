#include "QtHelpers/TableViewLogger.h"
#include "Configuration.h"
#include "Data/Logger.h"
#include "QtHelpers/ItemModelLoggerFields.h"
#include "QtHelpers/StyledItemDelegateColorPicker.h"
#include "pluginmain.h"
#include <QColorDialog>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QTextCodec>
#include <QUuid>
#include <nlohmann/json.hpp>

S2Plugin::TableViewLogger::TableViewLogger(Logger* logger, QWidget* parent) : QTableView(parent), mLogger(logger)
{
    setAlternatingRowColors(true);
    verticalHeader()->hide();
    horizontalHeader()->setStretchLastSection(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    mColorPickerDelegate = std::make_unique<StyledItemDelegateColorPicker>();
    setItemDelegateForColumn(gsLogFieldColColor, mColorPickerDelegate.get());

    QObject::connect(this, static_cast<void (QTableView::*)(const QModelIndex&)>(&QTableView::clicked), this, &TableViewLogger::cellClicked);
}

void S2Plugin::TableViewLogger::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("spelunky/memoryfield"))
    {
        event->acceptProposedAction();
    }
}

void S2Plugin::TableViewLogger::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat("spelunky/memoryfield"))
    {
        event->accept();
    }
}

void S2Plugin::TableViewLogger::dropEvent(QDropEvent* event)
{
    static const std::array defaultColors = {QColor(255, 102, 99), QColor(254, 177, 68), QColor(253, 253, 151), QColor(158, 224, 158), QColor(158, 193, 207), QColor(204, 153, 201)};

    auto fieldsModel = qobject_cast<ItemModelLoggerFields*>(model());

    auto data = event->mimeData()->data("spelunky/memoryfield");
    auto codec = QTextCodec::codecForName("UTF-8");
    auto str = codec->toUnicode(data);

    auto j = nlohmann::json::parse(str.toStdString());

    LoggerField field;
    field.type = static_cast<MemoryFieldType>(j[gsJSONDragDropMemoryField_Type].get<uint64_t>());
    switch (field.type)
    {
        // List allowed types
        case MemoryFieldType::Byte:
        case MemoryFieldType::UnsignedByte:
        case MemoryFieldType::Bool:
        case MemoryFieldType::Flags8:
        case MemoryFieldType::State8:
        case MemoryFieldType::CharacterDBID:
        case MemoryFieldType::Word:
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::Flags16:
        case MemoryFieldType::State16:
        case MemoryFieldType::Dword:
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::Float:
        case MemoryFieldType::Flags32:
        case MemoryFieldType::State32:
        case MemoryFieldType::EntityDBID:
        case MemoryFieldType::EntityUID:
        case MemoryFieldType::ParticleDBID:
        case MemoryFieldType::TextureDBID:
        case MemoryFieldType::StringsTableID:
        case MemoryFieldType::Qword:
        case MemoryFieldType::UnsignedQword:
        case MemoryFieldType::Double:
            break;
        default:
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowIcon(QIcon(":/icons/caveman.png"));
            msgBox.setText("This field type is not supported for logging");
            msgBox.setWindowTitle("Spelunky2");
            msgBox.exec();
            return;
        }
    }
    field.uuid = QUuid::createUuid().toString().toStdString();
    field.memoryOffset = j[gsJSONDragDropMemoryField_Offset].get<uint64_t>();
    field.name = j[gsJSONDragDropMemoryField_UID].get<std::string>();
    field.color = defaultColors[fieldsModel->rowCount() % defaultColors.size()];

    mLogger->addField(field);

    event->acceptProposedAction();
}

void S2Plugin::TableViewLogger::cellClicked(const QModelIndex& index)
{
    if (index.column() == gsLogFieldColColor)
    {
        auto currentColor = index.data().value<QColor>();
        auto newColor = QColorDialog::getColor(currentColor, this, "Pick a color");
        if (newColor.isValid())
        {
            mLogger->updateFieldColor(index.row(), newColor);
        }
    }
}

void S2Plugin::TableViewLogger::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        auto ix = selectedIndexes();
        if (ix.count() > 0)
        {
            auto& selectedIndex = ix.at(0);
            mLogger->removeFieldAt(selectedIndex.row());
            event->setAccepted(true);
        }
    }
}

void S2Plugin::TableViewLogger::paintEvent(QPaintEvent* event)
{
    QTableView::paintEvent(event);

    if (model()->rowCount() == 0)
    {
        QPainter painter(this->viewport());
        painter.save();

        static const auto caption = QString("Drag one or more memory fields here");
        static const auto font = QFont("Arial", 16);
        static const auto captionSize = QFontMetrics(font).size(Qt::TextSingleLine, caption);

        painter.setFont(font);
        painter.setPen(Qt::lightGray);
        painter.drawText(QPointF((width() / 2.) - (captionSize.width() / 2.), (height() / 2.) - (captionSize.height() / 2.)), caption);

        painter.restore();
    }
}
