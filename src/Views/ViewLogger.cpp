#include "Views/ViewLogger.h"
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>

S2Plugin::ViewLogger::ViewLogger(QWidget* parent) : QWidget(parent)
{
    mLogger = std::make_unique<Logger>();

    initializeUI();
    setWindowIcon(QIcon(":/icons/caveman.png"));
    setWindowTitle("Logger");
}

void S2Plugin::ViewLogger::closeEvent(QCloseEvent* event)
{
    delete this;
}

QSize S2Plugin::ViewLogger::sizeHint() const
{
    return QSize(650, 450);
}

QSize S2Plugin::ViewLogger::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewLogger::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(5);
    setLayout(mMainLayout);

    mTopLayout = new QHBoxLayout(this);
    mTopLayout->addWidget(new QLabel("Sample period:", this));
    mSamplePeriodLineEdit = new QLineEdit("8", this);
    mSamplePeriodLineEdit->setFixedWidth(50);
    mSamplePeriodLineEdit->setValidator(new QIntValidator(5, 5000, this));
    mTopLayout->addWidget(mSamplePeriodLineEdit);
    mTopLayout->addWidget(new QLabel("milliseconds", this));

    mTopLayout->addStretch();

    mTopLayout->addWidget(new QLabel("Duration:", this));
    mDurationLineEdit = new QLineEdit("5", this);
    mTopLayout->addWidget(mDurationLineEdit);
    mDurationLineEdit->setFixedWidth(50);
    mDurationLineEdit->setValidator(new QIntValidator(1, 500, this));
    mTopLayout->addWidget(new QLabel("seconds", this));

    mTopLayout->addStretch();

    mStartButton = new QPushButton(this);
    mStartButton->setText("Start");
    mTopLayout->addWidget(mStartButton);
    QObject::connect(mStartButton, &QPushButton::clicked, this, &ViewLogger::startLogging);

    mMainLayout->addLayout(mTopLayout);

    // TABS
    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mMainLayout->addWidget(mMainTabWidget);

    mTabFields = new QWidget();
    mTabSamples = new QWidget();
    mTabPlot = new QWidget();
    mTabFields->setLayout(new QVBoxLayout(mTabFields));
    mTabFields->layout()->setMargin(0);
    mTabSamples->setLayout(new QVBoxLayout(mTabSamples));
    mTabSamples->layout()->setMargin(0);
    mTabPlot->setLayout(new QVBoxLayout(mTabPlot));
    mTabPlot->layout()->setMargin(0);

    mMainTabWidget->addTab(mTabFields, "Fields");
    mMainTabWidget->addTab(mTabSamples, "Samples");
    mMainTabWidget->addTab(mTabPlot, "Plot");

    // TAB Fields
    {
        mFieldsTableView = new TableViewLogger(mLogger.get(), this);
        mFieldsTableView->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
        mFieldsTableView->setAcceptDrops(true);
        mTabFields->layout()->addWidget(mFieldsTableView);

        mFieldsTableModel = new ItemModelLoggerFields(mLogger.get(), mFieldsTableView);
        mLogger->setTableModel(mFieldsTableModel);
        mFieldsTableView->setModel(mFieldsTableModel);
        mFieldsTableView->setColumnWidth(gsLogFieldColColor, 45);
        mFieldsTableView->setColumnWidth(gsLogFieldColMemoryOffset, 125);
        mFieldsTableView->setColumnWidth(gsLogFieldColFieldType, 150);
    }

    // TAB Samples
    {
        mSamplesTableView = new QTableView(this);
        mTabSamples->layout()->addWidget(mSamplesTableView);
        mSamplesTableModel = new ItemModelLoggerSamples(mLogger.get(), mSamplesTableView);
        mSamplesTableView->setModel(mSamplesTableModel);
    }

    // TAB Plot
    {
        mSamplesPlotScroll = new QScrollArea(this);
        mSamplesPlotWidget = new WidgetSamplesPlot(mLogger.get(), this);
        mSamplesPlotScroll->setBackgroundRole(QPalette::Dark);
        mSamplesPlotScroll->setWidget(mSamplesPlotWidget);
        mSamplesPlotScroll->setWidgetResizable(true);
        mTabPlot->layout()->addWidget(mSamplesPlotScroll);
    }

    QObject::connect(mLogger.get(), &Logger::samplingEnded, this, &ViewLogger::samplingEnded);
    QObject::connect(mLogger.get(), &Logger::fieldsChanged, this, &ViewLogger::fieldsChanged);

    mSamplingWidget = new WidgetSampling(this);
    mSamplingWidget->setHidden(true);
    mMainLayout->addWidget(mSamplingWidget);
}

void S2Plugin::ViewLogger::startLogging()
{
    if (mLogger->fieldCount() > 0)
    {
        mSamplePeriodLineEdit->setEnabled(false);
        mDurationLineEdit->setEnabled(false);
        mStartButton->setEnabled(false);
        mMainTabWidget->setHidden(true);
        mSamplingWidget->setHidden(false);
        mLogger->start(mSamplePeriodLineEdit->text().toULongLong(), mDurationLineEdit->text().toULongLong());
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowIcon(QIcon(":/icons/caveman.png"));
        msgBox.setText("Please specify one or more fields to log");
        msgBox.setWindowTitle("Spelunky2");
        msgBox.exec();
    }
}

void S2Plugin::ViewLogger::samplingEnded()
{
    mSamplePeriodLineEdit->setEnabled(true);
    mDurationLineEdit->setEnabled(true);
    mStartButton->setEnabled(true);
    mSamplingWidget->setHidden(true);
    mMainTabWidget->setHidden(false);
    mSamplesTableModel->reset();
}

void S2Plugin::ViewLogger::fieldsChanged()
{
    mSamplesTableModel->reset();
}
