#include "BTUMainWindow.h"
#include "Logger.h"
#include <QApplication>
#include <QCloseEvent>
#include <QSplashScreen>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>

BTUMainWindow::BTUMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_isScanning(false)
    , m_isUninstalling(false)
    , m_settings(nullptr)
{
    // 设置窗口属性
    setWindowTitle(BTU_APP_DISPLAY_NAME);
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    // 初始化设置
    m_settings = new QSettings("BTU", "BoringToUninstall", this);
    
    // 创建核心组件
    m_scanner = new AppScanner(this);
    m_uninstallEngine = new UninstallEngine(this);
    
    // 创建定时器
    m_statusTimer = new QTimer(this);
    
    // 设置UI
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    
    // 加载设置
    loadSettings();
    
    LOG_INFO("BTU主窗口初始化完成");
    
    // 启动时自动扫描应用
    QTimer::singleShot(500, this, [this]() {
        onRefreshClicked();
    });
}

BTUMainWindow::~BTUMainWindow() {
    saveSettings();
    LOG_INFO("BTU主窗口正在关闭");
}

void BTUMainWindow::setupUI() {
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建顶部搜索区域
    m_topLayout = new QHBoxLayout();
    
    // 搜索框
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍 搜索应用程序...");
    m_searchEdit->setMinimumHeight(35);
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #ddd;"
        "    border-radius: 8px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #4CAF50;"
        "}"
    );
    
    // 刷新按钮
    m_refreshButton = new QPushButton("🔄 刷新", this);
    m_refreshButton->setMinimumHeight(35);
    m_refreshButton->setMinimumWidth(100);
    m_refreshButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #ccc;"
        "}"
    );
    
    m_topLayout->addWidget(m_searchEdit);
    m_topLayout->addWidget(m_refreshButton);
    
    // 创建应用列表表格
    m_appTable = new QTableWidget(this);
    m_appTable->setColumnCount(ColumnCount);
    
    QStringList headers;
    headers << "选择" << "应用名称" << "版本" << "发布商" << "大小" << "安装日期" << "安装位置";
    m_appTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_appTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_appTable->setAlternatingRowColors(true);
    m_appTable->setSortingEnabled(true);
    m_appTable->verticalHeader()->setVisible(false);
    
    // 设置列宽
    m_appTable->horizontalHeader()->setStretchLastSection(true);
    m_appTable->setColumnWidth(ColumnCheckBox, 60);
    m_appTable->setColumnWidth(ColumnName, 250);
    m_appTable->setColumnWidth(ColumnVersion, 100);
    m_appTable->setColumnWidth(ColumnPublisher, 150);
    m_appTable->setColumnWidth(ColumnSize, 80);
    m_appTable->setColumnWidth(ColumnInstallDate, 100);
    
    // 表格样式
    m_appTable->setStyleSheet(
        "QTableWidget {"
        "    gridline-color: #e0e0e0;"
        "    background-color: white;"
        "    alternate-background-color: #f9f9f9;"
        "    selection-background-color: #E3F2FD;"
        "    font-size: 13px;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "    border: none;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #2196F3;"
        "    color: white;"
        "}"
        "QHeaderView::section {"
        "    background-color: #f5f5f5;"
        "    padding: 10px;"
        "    border: 1px solid #ddd;"
        "    font-weight: bold;"
        "    font-size: 13px;"
        "}"
    );
    
    // 创建底部操作区域
    m_bottomLayout = new QHBoxLayout();
    
    // 全选复选框
    m_selectAllCheckBox = new QCheckBox("全选", this);
    m_selectAllCheckBox->setStyleSheet(
        "QCheckBox {"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QCheckBox::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "}"
    );
    
    // 反选按钮
    m_unselectAllButton = new QPushButton("🔄 反选", this);
    m_unselectAllButton->setMinimumHeight(35);
    m_unselectAllButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF9800;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F57C00;"
        "}"
    );
    
    // 卸载按钮
    m_uninstallButton = new QPushButton("🗑️ 深度卸载", this);
    m_uninstallButton->setMinimumHeight(35);
    m_uninstallButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #f44336;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d32f2f;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c62828;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #ccc;"
        "}"
    );
    
    // 退出按钮
    m_exitButton = new QPushButton("❌ 退出", this);
    m_exitButton->setMinimumHeight(35);
    m_exitButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #9E9E9E;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #757575;"
        "}"
    );
    
    // 选择信息标签
    m_selectionLabel = new QLabel("已选择 0 个应用", this);
    m_selectionLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #2196F3;"
        "}"
    );
    
    m_bottomLayout->addWidget(m_selectAllCheckBox);
    m_bottomLayout->addWidget(m_unselectAllButton);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_selectionLabel);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_uninstallButton);
    m_bottomLayout->addWidget(m_exitButton);
    
    // 添加到主布局
    m_mainLayout->addLayout(m_topLayout);
    m_mainLayout->addWidget(m_appTable);
    m_mainLayout->addLayout(m_bottomLayout);
    
    // 初始状态
    m_uninstallButton->setEnabled(false);
}

void BTUMainWindow::setupMenuBar() {
    m_menuBar = menuBar();
    
    // 文件菜单
    m_fileMenu = m_menuBar->addMenu("文件(&F)");
    
    m_refreshAction = new QAction("刷新应用列表(&R)", this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    m_fileMenu->addAction(m_refreshAction);
    
    m_fileMenu->addSeparator();
    
    m_exitAction = new QAction("退出(&X)", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_fileMenu->addAction(m_exitAction);
    
    // 查看菜单
    m_viewMenu = m_menuBar->addMenu("查看(&V)");
    
    m_viewLogAction = new QAction("查看日志(&L)", this);
    m_viewMenu->addAction(m_viewLogAction);
    
    // 帮助菜单
    m_helpMenu = m_menuBar->addMenu("帮助(&H)");
    
    m_aboutAction = new QAction("关于 BTU(&A)", this);
    m_helpMenu->addAction(m_aboutAction);
}

void BTUMainWindow::setupStatusBar() {
    // 状态标签
    m_statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(m_statusLabel);
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    statusBar()->addPermanentWidget(m_progressBar);
}

void BTUMainWindow::setupConnections() {
    // 界面信号连接
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BTUMainWindow::onSearchTextChanged);
    connect(m_refreshButton, &QPushButton::clicked, this, &BTUMainWindow::onRefreshClicked);
    connect(m_selectAllCheckBox, &QCheckBox::clicked, this, &BTUMainWindow::onSelectAllClicked);
    connect(m_unselectAllButton, &QPushButton::clicked, this, &BTUMainWindow::onUnselectAllClicked);
    connect(m_uninstallButton, &QPushButton::clicked, this, &BTUMainWindow::onUninstallClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &BTUMainWindow::onExitClicked);
    
    // 表格信号连接
    connect(m_appTable, &QTableWidget::itemChanged, this, &BTUMainWindow::onTableItemChanged);
    connect(m_appTable, &QTableWidget::itemSelectionChanged, this, &BTUMainWindow::onTableSelectionChanged);
    connect(m_appTable, &QTableWidget::itemDoubleClicked, this, &BTUMainWindow::onTableItemDoubleClicked);
    
    // 菜单信号连接
    connect(m_refreshAction, &QAction::triggered, this, &BTUMainWindow::onRefreshClicked);
    connect(m_exitAction, &QAction::triggered, this, &BTUMainWindow::onExitClicked);
    connect(m_viewLogAction, &QAction::triggered, this, &BTUMainWindow::onViewLogClicked);
    connect(m_aboutAction, &QAction::triggered, this, &BTUMainWindow::onAboutClicked);
    
    // 扫描器信号连接
    connect(m_scanner, &AppScanner::scanStarted, this, &BTUMainWindow::onScanStarted);
    connect(m_scanner, &AppScanner::scanFinished, this, &BTUMainWindow::onScanFinished);
    connect(m_scanner, &AppScanner::applicationFound, this, &BTUMainWindow::onApplicationFound);
    connect(m_scanner, &AppScanner::scanProgress, this, &BTUMainWindow::onScanProgress);
    connect(m_scanner, &AppScanner::scanError, this, &BTUMainWindow::onScanError);
    
    // 卸载引擎信号连接
    connect(m_uninstallEngine, &UninstallEngine::uninstallStarted, this, &BTUMainWindow::onUninstallStarted);
    connect(m_uninstallEngine, &UninstallEngine::uninstallFinished, this, &BTUMainWindow::onUninstallFinished);
    connect(m_uninstallEngine, &UninstallEngine::uninstallProgress, this, &BTUMainWindow::onUninstallProgress);
    connect(m_uninstallEngine, &UninstallEngine::uninstallError, this, &BTUMainWindow::onUninstallError);
    connect(m_uninstallEngine, &UninstallEngine::allUninstallsFinished, this, &BTUMainWindow::onAllUninstallsFinished);
    
    // 定时器信号连接
    connect(m_statusTimer, &QTimer::timeout, this, &BTUMainWindow::updateStatusInfo);
    m_statusTimer->start(1000); // 每秒更新一次状态
}

void BTUMainWindow::loadSettings() {
    // 加载窗口几何信息
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
    
    // 加载表格列宽
    for (int i = 0; i < ColumnCount; ++i) {
        int width = m_settings->value(QString("columnWidth_%1").arg(i), -1).toInt();
        if (width > 0) {
            m_appTable->setColumnWidth(i, width);
        }
    }
}

void BTUMainWindow::saveSettings() {
    // 保存窗口几何信息
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
    
    // 保存表格列宽
    for (int i = 0; i < ColumnCount; ++i) {
        m_settings->setValue(QString("columnWidth_%1").arg(i), m_appTable->columnWidth(i));
    }
}

void BTUMainWindow::closeEvent(QCloseEvent* event) {
    if (m_isScanning || m_isUninstalling) {
        QMessageBox::StandardButton ret = QMessageBox::question(this, "确认退出",
            "正在执行操作，确定要退出吗？\n未完成的操作将被中断。",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }
        
        // 停止正在进行的操作
        if (m_isScanning) {
            m_scanner->stopScan();
        }
        if (m_isUninstalling) {
            m_uninstallEngine->stopUninstall();
        }
    }
    
    saveSettings();
    event->accept();
}

// 继续实现其他方法...
void BTUMainWindow::onSearchTextChanged(const QString& text) {
    m_currentFilter = text;
    filterApplications();
}

void BTUMainWindow::onRefreshClicked() {
    if (m_isScanning) {
        return;
    }
    
    m_scanner->refreshApplications();
}

void BTUMainWindow::onSelectAllClicked() {
    bool selectAll = m_selectAllCheckBox->isChecked();
    
    for (int row = 0; row < m_appTable->rowCount(); ++row) {
        QTableWidgetItem* checkItem = m_appTable->item(row, ColumnCheckBox);
        if (checkItem) {
            checkItem->setCheckState(selectAll ? Qt::Checked : Qt::Unchecked);
        }
    }
    
    updateSelectionInfo();
}

void BTUMainWindow::onUnselectAllClicked() {
    m_selectAllCheckBox->setChecked(false);
    onSelectAllClicked();
}

void BTUMainWindow::onUninstallClicked() {
    QList<ApplicationInfo> selectedApps = getSelectedApplications();
    
    if (selectedApps.isEmpty()) {
        QMessageBox::information(this, "提示", "请至少选择一个应用程序进行卸载。");
        return;
    }
    
    // 显示确认对话框
    QString appNames;
    for (int i = 0; i < selectedApps.size() && i < 5; ++i) {
        appNames += "• " + selectedApps[i].name + "\n";
    }
    if (selectedApps.size() > 5) {
        appNames += QString("... 以及其他 %1 个应用").arg(selectedApps.size() - 5);
    }
    
    QMessageBox::StandardButton ret = QMessageBox::warning(this, "确认卸载",
        QString("确定要深度卸载以下 %1 个应用程序吗？\n\n%2\n"
                "⚠️ 警告：深度卸载将完全删除应用及其相关文件，此操作不可撤销！")
                .arg(selectedApps.size()).arg(appNames),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_uninstallEngine->uninstallApplications(selectedApps);
    }
}

void BTUMainWindow::onExitClicked() {
    close();
}

void BTUMainWindow::onScanStarted() {
    m_isScanning = true;
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定进度
    m_statusLabel->setText("正在扫描已安装应用...");
    setUIEnabled(false);
    
    // 清空表格
    m_appTable->setRowCount(0);
    m_allApplications.clear();
    m_filteredApplications.clear();
    
    LOG_INFO("开始扫描应用程序列表");
}

void BTUMainWindow::onScanFinished() {
    m_isScanning = false;
    m_progressBar->setVisible(false);
    m_statusLabel->setText(QString("扫描完成，找到 %1 个应用程序").arg(m_allApplications.size()));
    setUIEnabled(true);
    
    updateSelectionInfo();
    LOG_INFO(QString("应用程序扫描完成，共找到 %1 个应用").arg(m_allApplications.size()));
}

void BTUMainWindow::onApplicationFound(const ApplicationInfo& appInfo) {
    m_allApplications.append(appInfo);
    addApplicationToTable(appInfo);
}

void BTUMainWindow::onScanProgress(int current, int total) {
    if (total > 0) {
        m_progressBar->setRange(0, total);
        m_progressBar->setValue(current);
    }
    m_statusLabel->setText(QString("正在扫描应用程序... (%1/%2)").arg(current).arg(total));
}

void BTUMainWindow::onScanError(const QString& error) {
    QMessageBox::warning(this, "扫描错误", QString("扫描过程中发生错误：\n%1").arg(error));
    LOG_ERROR(QString("扫描错误: %1").arg(error));
}

void BTUMainWindow::addApplicationToTable(const ApplicationInfo& appInfo) {
    // 应用过滤器
    if (!m_currentFilter.isEmpty()) {
        if (!appInfo.name.contains(m_currentFilter, Qt::CaseInsensitive) &&
            !appInfo.publisher.contains(m_currentFilter, Qt::CaseInsensitive)) {
            return;
        }
    }
    
    int row = m_appTable->rowCount();
    m_appTable->insertRow(row);
    
    // 复选框
    QTableWidgetItem* checkItem = new QTableWidgetItem();
    checkItem->setCheckState(Qt::Unchecked);
    checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    m_appTable->setItem(row, ColumnCheckBox, checkItem);
    
    // 应用名称
    QTableWidgetItem* nameItem = new QTableWidgetItem(appInfo.displayName);
    nameItem->setData(Qt::UserRole, QVariant::fromValue(appInfo));
    if (appInfo.isSystemApp) {
        nameItem->setForeground(QBrush(QColor(255, 87, 34))); // 橙色表示系统应用
        nameItem->setToolTip("系统关键应用，建议不要卸载");
    }
    m_appTable->setItem(row, ColumnName, nameItem);
    
    // 版本
    m_appTable->setItem(row, ColumnVersion, new QTableWidgetItem(appInfo.version));
    
    // 发布商
    m_appTable->setItem(row, ColumnPublisher, new QTableWidgetItem(appInfo.publisher));
    
    // 大小
    m_appTable->setItem(row, ColumnSize, new QTableWidgetItem(appInfo.estimatedSize));
    
    // 安装日期
    m_appTable->setItem(row, ColumnInstallDate, new QTableWidgetItem(appInfo.installDate));
    
    // 安装位置
    m_appTable->setItem(row, ColumnLocation, new QTableWidgetItem(appInfo.installLocation));
}

void BTUMainWindow::filterApplications() {
    m_appTable->setRowCount(0);
    
    for (const ApplicationInfo& appInfo : m_allApplications) {
        addApplicationToTable(appInfo);
    }
    
    updateSelectionInfo();
}

QList<ApplicationInfo> BTUMainWindow::getSelectedApplications() const {
    QList<ApplicationInfo> selectedApps;
    
    for (int row = 0; row < m_appTable->rowCount(); ++row) {
        QTableWidgetItem* checkItem = m_appTable->item(row, ColumnCheckBox);
        if (checkItem && checkItem->checkState() == Qt::Checked) {
            QTableWidgetItem* nameItem = m_appTable->item(row, ColumnName);
            if (nameItem) {
                ApplicationInfo appInfo = nameItem->data(Qt::UserRole).value<ApplicationInfo>();
                selectedApps.append(appInfo);
            }
        }
    }
    
    return selectedApps;
}

void BTUMainWindow::updateSelectionInfo() {
    QList<ApplicationInfo> selectedApps = getSelectedApplications();
    m_selectionLabel->setText(QString("已选择 %1 个应用").arg(selectedApps.size()));
    m_uninstallButton->setEnabled(!selectedApps.isEmpty() && !m_isUninstalling);
}

void BTUMainWindow::setUIEnabled(bool enabled) {
    m_searchEdit->setEnabled(enabled);
    m_refreshButton->setEnabled(enabled);
    m_selectAllCheckBox->setEnabled(enabled);
    m_unselectAllButton->setEnabled(enabled);
    m_uninstallButton->setEnabled(enabled && !getSelectedApplications().isEmpty());
    m_appTable->setEnabled(enabled);
}

void BTUMainWindow::onTableItemChanged(QTableWidgetItem* item) {
    if (item && item->column() == ColumnCheckBox) {
        updateSelectionInfo();
    }
}

void BTUMainWindow::onTableSelectionChanged() {
    // 这个方法可以用于处理表格选择变化
}

void BTUMainWindow::onTableItemDoubleClicked(QTableWidgetItem* item) {
    if (item) {
        QTableWidgetItem* nameItem = m_appTable->item(item->row(), ColumnName);
        if (nameItem) {
            ApplicationInfo appInfo = nameItem->data(Qt::UserRole).value<ApplicationInfo>();
            showApplicationDetails(appInfo);
        }
    }
}

void BTUMainWindow::showApplicationDetails(const ApplicationInfo& appInfo) {
    QString details = QString(
        "应用程序详细信息\n"
        "===================\n"
        "名称: %1\n"
        "版本: %2\n"
        "发布商: %3\n"
        "大小: %4\n"
        "安装日期: %5\n"
        "安装位置: %6\n"
        "卸载命令: %7\n"
        "注册表键: %8\n"
        "系统应用: %9"
    ).arg(appInfo.displayName)
     .arg(appInfo.version.isEmpty() ? "未知" : appInfo.version)
     .arg(appInfo.publisher.isEmpty() ? "未知" : appInfo.publisher)
     .arg(appInfo.estimatedSize)
     .arg(appInfo.installDate.isEmpty() ? "未知" : appInfo.installDate)
     .arg(appInfo.installLocation.isEmpty() ? "未知" : appInfo.installLocation)
     .arg(appInfo.uninstallString.isEmpty() ? "无" : appInfo.uninstallString)
     .arg(appInfo.registryKey)
     .arg(appInfo.isSystemApp ? "是" : "否");
    
    QMessageBox::information(this, "应用程序详情", details);
}

void BTUMainWindow::onUninstallStarted(const QString& appName) {
    m_isUninstalling = true;
    m_statusLabel->setText(QString("正在卸载: %1").arg(appName));
    setUIEnabled(false);
    LOG_INFO(QString("开始卸载应用: %1").arg(appName));
}

void BTUMainWindow::onUninstallFinished(const QString& appName, UninstallResult result) {
    QString resultText = formatUninstallResult(result);
    LOG_INFO(QString("应用 %1 卸载完成: %2").arg(appName, resultText));
}

void BTUMainWindow::onUninstallProgress(const QString& appName, const UninstallProgress& progress) {
    m_statusLabel->setText(QString("卸载 %1: %2").arg(appName, progress.currentOperation));
}

void BTUMainWindow::onUninstallError(const QString& appName, const QString& error) {
    LOG_ERROR(QString("卸载 %1 时发生错误: %2").arg(appName, error));
}

void BTUMainWindow::onAllUninstallsFinished() {
    m_isUninstalling = false;
    m_statusLabel->setText("所有卸载操作完成");
    setUIEnabled(true);
    
    // 刷新应用列表
    QTimer::singleShot(1000, this, [this]() {
        onRefreshClicked();
    });
    
    QMessageBox::information(this, "卸载完成", "所有选中的应用程序已处理完毕！");
}

QString BTUMainWindow::formatUninstallResult(UninstallResult result) const {
    switch (result) {
        case UninstallResult::Success: return "成功";
        case UninstallResult::Failed: return "失败";
        case UninstallResult::Cancelled: return "已取消";
        case UninstallResult::PartialSuccess: return "部分成功";
        default: return "未知";
    }
}

void BTUMainWindow::onAboutClicked() {
    QString aboutText = QString(
        "<h2>%1</h2>"
        "<p><b>版本:</b> %2</p>"
        "<p><b>描述:</b> %3</p>"
        "<p><b>版权:</b> %4</p>"
        "<br>"
        "<p>这是一个Windows深度卸载工具，能够完全删除应用程序及其相关文件，"
        "包括注册表项、用户数据、临时文件等，让卸载不再无聊！</p>"
        "<br>"
        "<p><b>特性:</b></p>"
        "<ul>"
        "<li>安全的应用程序检测</li>"
        "<li>深度清理残留文件</li>"
        "<li>保护系统关键组件</li>"
        "<li>批量卸载支持</li>"
        "<li>详细的操作日志</li>"
        "</ul>"
    ).arg(BTU_APP_DISPLAY_NAME)
     .arg(BTU_VERSION_STRING)
     .arg(BTU_APP_DESCRIPTION)
     .arg(BTU_APP_COPYRIGHT);
    
    QMessageBox::about(this, "关于 BTU", aboutText);
}

void BTUMainWindow::onSettingsClicked() {
    // 设置对话框可以在后续版本中实现
    QMessageBox::information(this, "设置", "设置功能将在后续版本中提供。");
}

void BTUMainWindow::onViewLogClicked() {
    QString logContent = Logger::instance().getLogContent();
    
    QDialog* logDialog = new QDialog(this);
    logDialog->setWindowTitle("日志查看器");
    logDialog->setMinimumSize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(logDialog);
    
    QTextEdit* logTextEdit = new QTextEdit(logDialog);
    logTextEdit->setPlainText(logContent);
    logTextEdit->setReadOnly(true);
    logTextEdit->setFont(QFont("Consolas", 10));
    
    QPushButton* clearButton = new QPushButton("清空日志", logDialog);
    QPushButton* closeButton = new QPushButton("关闭", logDialog);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    layout->addWidget(logTextEdit);
    layout->addLayout(buttonLayout);
    
    connect(clearButton, &QPushButton::clicked, [logTextEdit]() {
        Logger::instance().clearLog();
        logTextEdit->clear();
    });
    
    connect(closeButton, &QPushButton::clicked, logDialog, &QDialog::accept);
    
    logDialog->exec();
    logDialog->deleteLater();
}

void BTUMainWindow::updateStatusInfo() {
    // 定期更新状态信息
    if (!m_isScanning && !m_isUninstalling) {
        QString memoryInfo = QString("内存使用: %1 MB").arg(
            QCoreApplication::applicationPid()
        );
        // 可以在这里添加更多状态信息
    }
}

// 需要包含moc文件
#include "BTUMainWindow.moc"
