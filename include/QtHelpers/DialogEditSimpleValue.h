#pragma once

#include "Spelunky2.h"
#include <QDialog>
#include <QLineEdit>

namespace S2Plugin
{
    class DialogEditSimpleValue : public QDialog
    {
        Q_OBJECT

      public:
        DialogEditSimpleValue(const QString& fieldName, size_t memoryOffset, MemoryFieldType type, QWidget* parent = nullptr);

      protected:
        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      private slots:
        void cancelBtnClicked();
        void changeBtnClicked();
        void decValueChanged(const QString& text);

      private:
        size_t mMemoryOffset;
        MemoryFieldType mFieldType;

        QLineEdit* mLineEditDecValue;
        QLineEdit* mLineEditHexValue;
    };
} // namespace S2Plugin