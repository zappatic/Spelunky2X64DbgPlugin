#include "Views/ViewJournalPage.h"
#include "Configuration.h"
#include "Data/JournalPage.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewJournalPage::ViewJournalPage(ViewToolbar* toolbar, size_t offset, const std::string& pageType, QWidget* parent) : QWidget(parent), mOffset(offset), mPageType(pageType), mToolbar(toolbar)
{
    mJournalPage = std::make_unique<JournalPage>(mOffset, mPageType);

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("JournalPage");

    // mMainTreeView->setMemoryMappedData(mJournalPage.get());

    refreshJournalPage();
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffset, 125);
    mMainTreeView->setColumnWidth(gsColMemoryOffsetDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewJournalPage::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout(this);
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewJournalPage::refreshJournalPage);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewJournalPage::autoRefreshTimerTrigger);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewJournalPage::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewJournalPage::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    mRefreshLayout->addWidget(new QLabel("Interpret as:", this));
    mInterpretAsComboBox = new QComboBox(this);
    mInterpretAsComboBox->addItem("JournalPage");
    mInterpretAsComboBox->addItem("JournalPageProgress");
    mInterpretAsComboBox->addItem("JournalPageJournalMenu");
    mInterpretAsComboBox->addItem("JournalPagePlaces");
    mInterpretAsComboBox->addItem("JournalPagePeople");
    mInterpretAsComboBox->addItem("JournalPageBestiary");
    mInterpretAsComboBox->addItem("JournalPageItems");
    mInterpretAsComboBox->addItem("JournalPageTraps");
    mInterpretAsComboBox->addItem("JournalPageStory");
    mInterpretAsComboBox->addItem("JournalPageFeats");
    mInterpretAsComboBox->addItem("JournalPageDeathCause");
    mInterpretAsComboBox->addItem("JournalPageDeathMenu");
    mInterpretAsComboBox->addItem("JournalPageRecap");
    mInterpretAsComboBox->addItem("JournalPagePlayerProfile");
    mInterpretAsComboBox->addItem("JournalPageLastGamePlayed");

    QObject::connect(mInterpretAsComboBox, &QComboBox::currentTextChanged, this, &ViewJournalPage::interpretAsChanged);
    mRefreshLayout->addWidget(mInterpretAsComboBox);
    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewJournalPage::label);
    mRefreshLayout->addWidget(labelButton);

    mMainTreeView = new TreeViewMemoryFields(mToolbar, this);
    for (const auto& field : Configuration::get()->typeFieldsOfInlineStruct(mPageType))
    {
        mMainTreeView->addMemoryField(field, mPageType + "." + field.name);
    }
    mMainTreeView->setColumnHidden(gsColComparisonValue, true);
    mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->updateTableHeader();

    mMainLayout->setMargin(5);
    setLayout(mMainLayout);
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewJournalPage::closeEvent(QCloseEvent* event)
{
    delete this;
}

void S2Plugin::ViewJournalPage::refreshJournalPage()
{
    mJournalPage->refreshOffsets();
    auto& offsets = mJournalPage->offsets();
    auto deltaReference = offsets.at(mPageType + ".__vftable");
    for (const auto& field : Configuration::get()->typeFieldsOfInlineStruct(mPageType))
    {
        mMainTreeView->updateValueForField(field, mPageType + "." + field.name, offsets, deltaReference);
    }
}

void S2Plugin::ViewJournalPage::toggleAutoRefresh(int newState)
{
    if (newState == Qt::Unchecked)
    {
        mAutoRefreshTimer->stop();
        mRefreshButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::ViewJournalPage::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
    }
}

void S2Plugin::ViewJournalPage::autoRefreshTimerTrigger()
{
    refreshJournalPage();
}

QSize S2Plugin::ViewJournalPage::sizeHint() const
{
    return QSize(750, 750);
}

QSize S2Plugin::ViewJournalPage::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewJournalPage::label()
{
    for (const auto& [fieldName, offset] : mJournalPage->offsets())
    {
        DbgSetAutoLabelAt(offset, fieldName.c_str());
    }
}

void S2Plugin::ViewJournalPage::interpretAsChanged(const QString& text)
{
    if (!text.isEmpty())
    {
        auto textStr = text.toStdString();
        mJournalPage->interpretAs(textStr);
        mPageType = textStr;
        mMainTreeView->clear();
        for (const auto& field : Configuration::get()->typeFieldsOfInlineStruct(mPageType))
        {
            mMainTreeView->addMemoryField(field, mPageType + "." + field.name);
        }
        mMainTreeView->setColumnHidden(gsColComparisonValue, true);
        mMainTreeView->setColumnHidden(gsColComparisonValueHex, true);
        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->updateTableHeader();
        refreshJournalPage();
        mInterpretAsComboBox->setCurrentText("");
    }
}
