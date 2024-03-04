#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <array>

namespace S2Plugin
{
    class ViewToolbar;
    class TreeViewMemoryFields;

    enum class MASK : uint32_t
    {
        PLAYER = 0x1,
        MOUNT = 0x2,
        MONSTER = 0x4,
        ITEM = 0x8,
        EXPLOSION = 0x10,
        ROPE = 0x20,
        FX = 0x40,
        ACTIVEFLOOR = 0x80,
        FLOOR = 0x100,
        DECORATION = 0x200,
        BG = 0x400,
        SHADOW = 0x800,
        LOGICAL = 0x1000,
        WATER = 0x2000,
        LAVA = 0x4000
    };
    class ViewEntities : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntities(ViewToolbar* toolbar, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshEntities();

      private:
        struct EntitySelectSlot
        {
            QCheckBox* mCheckbox;
            const MASK mask;
            const QString name;
        };
        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;

        QCheckBox* mCheckboxLayer0;
        QCheckBox* mCheckboxLayer1;

        std::array<EntitySelectSlot, 15> mCheckbox = {
            EntitySelectSlot{nullptr, MASK::PLAYER, QString{"PLAYER"}},
            EntitySelectSlot{nullptr, MASK::MOUNT, QString{"MOUNT"}},
            EntitySelectSlot{nullptr, MASK::MONSTER, QString{"MONSTER"}},
            EntitySelectSlot{nullptr, MASK::ITEM, QString{"ITEM"}},
            EntitySelectSlot{nullptr, MASK::EXPLOSION, QString{"EXPLOSION"}},
            EntitySelectSlot{nullptr, MASK::ROPE, QString{"ROPE"}},
            EntitySelectSlot{nullptr, MASK::FX, QString{"FX"}},
            EntitySelectSlot{nullptr, MASK::ACTIVEFLOOR, QString{"ACTIVEFLOOR"}},
            EntitySelectSlot{nullptr, MASK::FLOOR, QString{"FLOOR"}},
            EntitySelectSlot{nullptr, MASK::DECORATION, QString{"DECORATION"}},
            EntitySelectSlot{nullptr, MASK::BG, QString{"BG"}},
            EntitySelectSlot{nullptr, MASK::SHADOW, QString{"SHADOW"}},
            EntitySelectSlot{nullptr, MASK::LOGICAL, QString{"LOGICAL"}},
            EntitySelectSlot{nullptr, MASK::WATER, QString{"WATER"}},
            EntitySelectSlot{nullptr, MASK::LAVA, QString{"LAVA"}},
        };

        QLineEdit* mFilterLineEdit;

        ViewToolbar* mToolbar;
        uintptr_t mLayer0Offset = 0;
        uintptr_t mLayer1Offset = 0;
        uintptr_t mLayerMapOffset = 0;

        void initializeTreeView();
        void initializeRefreshAndFilter();
    };
} // namespace S2Plugin
